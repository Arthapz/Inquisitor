// Copryright (C) 2021 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

/////////// - STL - ///////////
#include <csignal>

/////////// - Inquisitor - ///////////
#include "Inquisitor.hpp"
#include "Log.hpp"

namespace beast = boost::beast;
namespace ip = beast::net::ip;
namespace http = beast::http;
using tcp = ip::tcp;

static auto urlEncode(std::string_view data) -> std::string {
    auto result = std::string{};
    result.reserve(std::size(data));

    for(auto c : data) {
        if(std::isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '_' || c == '.' || c == '~') result += c;
        else {
            result += '%';
            result += fmt::format("{:0X}", c);
        }
    }

    return result;
}

static auto curlWriteData(void *ptr, std::size_t size, std::size_t nmemb, void *user_data) -> std::size_t {
    auto &data = *reinterpret_cast<std::string*>(user_data);

    data.reserve(std::size(data) + size * nmemb);

    auto c_data = std::span<const char>{reinterpret_cast<const char *>(ptr), nmemb};
    std::ranges::copy(c_data, std::back_inserter(data));

    return size * nmemb;
}

/////////////////////////////////////
/////////////////////////////////////
Inquisitor::Inquisitor() noexcept {
    ilog("Using StormKit {}.{}.{} {} {}",
         STORMKIT_MAJOR_VERSION,
         STORMKIT_MINOR_VERSION,
         STORMKIT_PATCH_VERSION,
         STORMKIT_GIT_BRANCH,
         STORMKIT_GIT_COMMIT_HASH);

    curl_global_init(CURL_GLOBAL_ALL);
    parseSettings();
    loadPlugins();
    initializeBot();
}

/////////////////////////////////////
/////////////////////////////////////
Inquisitor::~Inquisitor() {
    for (auto &plugin : m_plugins) {
        auto deallocate_func = plugin.module.getFunc<void(PluginInterface *)>("deallocatePlugin");
        deallocate_func(plugin.interface);
    }

    curl_global_cleanup();
}

/////////////////////////////////////
/////////////////////////////////////
auto Inquisitor::run([[maybe_unused]] const int argc, [[maybe_unused]] const char **argv) -> void {
    m_bot->run();
}

/////////////////////////////////////
/////////////////////////////////////
auto Inquisitor::parseSettings() -> void {
    ilog("Loading settings.json");

    auto file = std::ifstream {};
    file.open("settings.json");

    const auto string =
        std::string { std::istreambuf_iterator<char> { file }, std::istreambuf_iterator<char> {} };

    auto document = json::parse(string);

    m_token = document["token"].get<std::string>();

    auto enabled_plugins = document["enabled_plugins"].get<std::vector<std::string>>();
    for(auto &enabled_plugin : enabled_plugins) {
        m_enabled_plugins.emplace_back(enabled_plugin);

        auto options = json{};

        if(document.contains(enabled_plugin))
            options = document[enabled_plugin];

        options["inquisitor"] = json::parse(fmt::format(R"({{ "major": {}, "minor": {} }})", MAJOR_VERSION, MINOR_VERSION));

        m_plugin_options.emplace(enabled_plugin, std::move(options));
    }
}

/////////////////////////////////////
/////////////////////////////////////
auto Inquisitor::loadPlugins() -> void {
    for (auto &plugin : std::filesystem::recursive_directory_iterator("plugins")) {
        const auto plugin_path = plugin.path();

        if (plugin_path.extension() == ".so") {
            ilog("{} found", plugin_path.string());
            loadPlugin(plugin_path);
        }
    }
}

/////////////////////////////////////
/////////////////////////////////////
auto Inquisitor::loadPlugin(const std::filesystem::path &path) -> void {
    auto module = storm::module::Module { path };

    if (!module.isLoaded()) elog("Failed to load plugin {}", path.string());

    auto allocate_func = module.getFunc<PluginInterface *()>("allocatePlugin");

    auto plugin_interface = allocate_func();

    const auto plugin_name = plugin_interface->name();

    if(std::ranges::find(m_enabled_plugins, plugin_interface->name()) == std::ranges::cend(m_enabled_plugins)) return;

    ilog("{} loaded", plugin_name);

    m_plugins.emplace_back(Plugin { std::move(path), std::move(module), plugin_interface });
}

/////////////////////////////////////
/////////////////////////////////////
auto Inquisitor::initializeBot() -> void {

#ifdef STORMKIT_BUILD_DEBUG
    discordpp::log::filter = discordpp::log::debug;
#else
    discordpp::log::filter = discordpp::log::info;
#endif

    discordpp::log::out = &std::cerr;


    m_asio_context = std::make_shared<boost::asio::io_context>();
    m_bot = std::make_shared<Bot>();
    m_bot->intents = discordpp::intents::GUILD_MESSAGES | discordpp::intents::DIRECT_MESSAGE_REACTIONS | discordpp::intents::GUILDS;
    m_bot->debugUnhandled = false;
    m_bot->prefix = ";inquisitor ";

    m_bot->handlers.emplace(std::string{"READY"}, std::function<void(json)>{[this](json data){
                                ilog("Connected !");
                                for(auto &plugin : m_plugins) {
                                    plugin.interface->onReady(data);
                                }
                            }});

    m_bot->handlers.emplace(std::string{"MESSAGE_CREATE"}, std::function<void(json)>{[this](json data){
                                for(auto &plugin : m_plugins) {
                                    plugin.interface->onMessageReceived(data);
                                }
                            }});

    auto plugins = std::vector<const PluginInterface *>{};
    for(const auto &plugin :m_plugins)
        plugins.emplace_back(plugin.interface);

    const auto send_message = [this](std::string_view channel_id,
                                     const json &msg) {
        m_bot->callJson()
            ->method("POST")
            ->target(fmt::format("/channels/{}/messages", channel_id))
            ->payload(msg)
            ->onRead([channel_id](const bool error, [[maybe_unused]] const json msg) {
                if(error)
                    elog("Failed to send message to channel {}", channel_id);
            })->run();
    };

    const auto send_file = [this](std::string_view channel_id,
                                  std::string filename,
                                  std::string filetype,
                                  std::string file,
                                  const json &msg) {
        m_bot->callFile()
            ->method("POST")
            ->target(fmt::format("/channels/{}/messages", channel_id))
            ->filename(std::move(filename))
            ->filetype(std::move(filetype))
            ->file(std::move(file))
            ->payload(msg)
            ->onRead([channel_id](const bool error, [[maybe_unused]] const json msg) {
                if(error)
                    elog("Failed to send file to channel {}", channel_id);
            })
            ->run();

    };

    const auto get_message = [this](std::string_view channel_id,
                                    std::string_view message_id,
                                    std::function<void(const json &)> on_response) {
        m_bot->call()
            ->method("GET")
            ->target(fmt::format("/channels/{}/messages/{}", channel_id, message_id))
            ->onRead(
                [on_response = std::move(on_response), channel_id, message_id](const bool error, const json msg) {
                    if(error) {
                        elog("Failed to get message {} on channel {}, retrying", message_id, channel_id);
                        return;
                    }
                    on_response(msg);
                })->run();
    };

    const auto get_channel = [this](std::string_view channel_id,
                                    std::function<void(const json &)> on_response) {
        m_bot->call()
            ->method("GET")
            ->target(fmt::format("/channels/{}", channel_id))
            ->onRead([on_response = std::move(on_response), channel_id](const bool error, const json msg) {
                if(error) {
                    elog("Failed to get channel {}, retrying", channel_id);
                    return;
                }

                on_response(msg);
            })->run();

    };

    const auto get_all_message = [this](std::string_view channel_id, std::function<void(const json &)> on_response) {
        auto payload = json{};
        //auto payload = json{
        //    { "limit", 100 }
        //};
        //ilog("{}", payload.dump());
        m_bot->callJson()
            ->method("GET")
            ->target(fmt::format("/channels/{}/messages", channel_id))
            ->payload(std::move(payload))
            ->onRead([on_response = std::move(on_response), channel_id](const bool error, const json msg) {
                if(error) {
                    elog("Failed to get all messages from channel {}", channel_id);
                    return;
                }

                on_response(msg);
            })->run();
    };

    const auto delete_message = [this](std::string_view channel_id, std::string_view message_id) {
        auto payload = json{};

        m_bot->callJson()
            ->method("DELETE")
            ->target(fmt::format("/channels/{}/messages/{}", channel_id, message_id))
            ->payload(std::move(payload))
            ->onRead([channel_id, message_id](const bool error, [[maybe_unused]] const json msg) {
                if(error)
                    elog("Failed to get delete message {} from channel {}, retrying", message_id, channel_id);
            })->run();
    };

    const auto delete_messages = [this](std::string_view channel_id, std::span<const std::string> message_ids) {
        auto payload = json {
            { "messages", message_ids }
        };

        m_bot->callJson()
            ->method("POST")
            ->target(fmt::format("/channels/{}/messages/bulk-delete", channel_id))
            ->payload(std::move(payload))
            ->onRead([channel_id](const bool error, [[maybe_unused]] const json msg) {
                if(error)
                    elog("Failed to get delete all messages from channel {}, retrying", channel_id);
            })->run();
    };

    const auto add_reaction = [this](std::string_view channel_id, std::string_view message_id, std::string_view emoji) {
        auto payload = json{};

        m_bot->callJson()
            ->method("PUT")
            ->target(fmt::format("/channels/{}/messages/{}/reactions/{}/@me", channel_id, message_id, emoji))
            ->payload(std::move(payload))
            ->onRead([channel_id, message_id](const bool error, [[maybe_unused]] const json msg) {
                if(error)
                    elog("Failed to add reaction messages to message {} on channel {}, retrying", message_id, channel_id);
            })->run();
    };

    const auto get_http_file = [](std::string_view url) {
        auto curl = curl_easy_init();

        auto data = std::string{};

        curl_easy_setopt(curl, CURLOPT_URL, std::data(url));
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteData);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
        curl_easy_perform(curl);

        curl_easy_cleanup(curl);

        return data;
    };

    for(auto &plugin : m_plugins) {
        plugin.interface->initialize(PluginInterface::Functions {send_message,
                                         send_file,
                                         get_message,
                                         get_channel,
                                         get_all_message,
                                         delete_message,
                                         delete_messages,
                                         add_reaction,
                                         get_http_file
                                     },m_plugin_options.at(std::string{plugin.interface->name()}), plugins);

        for(auto command : plugin.interface->commands()) {
            m_bot->respond(std::string{command},
                           [command, &plugin](json data) {
                               plugin.interface->onCommand(command, data);
                           });
        }
    }

    m_bot->initBot(9, "Bot " + m_token, m_asio_context);
}
