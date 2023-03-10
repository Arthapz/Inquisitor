// Copryright (C) 2023 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

#include "Inquisitor.hpp"

#include <curl/curl.h>

#pragma push_macro("interface")
#undef interface

using namespace std::literals;
using namespace stormkit;

static constexpr auto ASCII_ART_LOGO =
    "\nmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm"
    "mmmmmmmm"
    "mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm"
    "mmmmmm"
    "mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmhyhhhhhhhyhmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm"
    "mmmmmm"
    "mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm+dmmmmmmmd+"
    "mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm"
    "mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmysmmmmmmmmmshmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm"
    "mmmmmm"
    "mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm+dmmmmmmmmmd+"
    "mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm"
    "mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmysmmmmmmmmmmmsymmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm"
    "mmmmmm"
    "mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm+dmmmmmmmmmmmm+"
    "mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm"
    "mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmhommmmmmmmmmmmmsymmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm"
    "mmmmmm"
    "mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm+dmmmmmmmmmmmmmm+"
    "mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm"
    "mmmmmmdddddddmmmmdddmmmmmddddddmmmddddddy+"
    "dmmmmdhysydmmmmssddddmmmmmddddddddmdddmmmmmddmmmmddddmmmmm"
    "mmmmm/       +mm+   ymmmy     `-sm/        dmh/`     .+mo     `-smmm       `m-  -mmmy .dmmy   "
    "ymmmmm"
    "mmmmmo::   ::smd`   .mmmy   ::`  so:::`   /ms   -:o+. .so  `::   smm   -:::/md`  +md.  :md`  "
    "+mmmmmm"
    "mmmmmmmh   dmmm:     /mmy   ss.  +mmm/  `ymm`  om/hmmhmmo  `oo`  omm   -::::mmy   y:    s-  "
    ".mmmmmmm"
    "mmmmmmmh   dmmy   o   hmy       :mmy.  -dmmm   yyo+mmmmmo       /mmm        mmm/           "
    "`hmmmmmmm"
    "mmmmmmmh   dmm.  /m/  .my   oosdmmo   `+sssm+  `-s/o:./ho  `/`  .hmm   /ssssmmmm.    o-    "
    "smmmmmmmm"
    "mmmmmmmh   dmo  `dmm.  +y   mmmmmm/        hms.      `:do  .h+:  `ym        mmmmh`  :mh   "
    ":mmmmmmmmm"
    "mmmmmmmmsssmmysshmmmhsssdsssmmmmmmyss:ossssmmmd/"
    "o++:+dmmdssymohssssmssssssssmmmmmyssdmmyssdmmmmmmmmm"
    "mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmd+mmmmmmmmysmmmd+mmmmmmmmd+"
    "mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm"
    "mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmshmmmmmmmm+"
    "dmmmmsymmmmmmmmohmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm"
    "mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmd+mmmmmmmmhsmmmmmm+mmmmmmmmd+"
    "mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm"
    "mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmsymmmmmmmm+"
    "dmmmmmmysmmmmmmmmsymmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm"
    "mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmd+mmmmmmmmhommmmmmmm+dmmmmmmmd+"
    "mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm"
    "mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmsymmmmmmmmodmmmmmmmmysmmmmmmmmsymmmmmmmmmmmmmmmmmmmmmmmmmmmm"
    "mmmmmm"
    "mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm+mmmmmmmmhommmmmmmmmm+dmmmmmmmm+"
    "mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm"
    "mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmhoyyyyyyyyohmmmmmmmmmmhsyyyyyyyyohmmmmmmmmmmmmmmmmmmmmmmmmmmm"
    "mmmmmm"
    "mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm"
    "mmmmmm"
    "mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm"sv;

/*static auto urlEncode(std::string_view data) -> std::string {
    auto result = std::string {};
    result.reserve(std::size(data));

    for (auto c : data) {
        if (std::isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '_' || c == '.' ||
            c == '~')
            result += c;
        else {
            result += '%';
            result += std::format("{:0X}", c);
        }
    }

    return result;
}

static auto curlWriteData(void *ptr, std::size_t size, std::size_t nmemb, void *user_data)
    -> std::size_t {
    auto &data = *reinterpret_cast<std::string *>(user_data);

    data.reserve(std::size(data) + size * nmemb);

    auto c_data = std::span<const char> { reinterpret_cast<const char *>(ptr), nmemb };
    std::ranges::copy(c_data, std::back_inserter(data));

    return size * nmemb;
}*/

/////////////////////////////////////
/////////////////////////////////////
Inquisitor::Inquisitor() noexcept {
    core::print(ASCII_ART_LOGO);
    ilog("Using StormKit {}.{}.{} {} {}",
         core::STORMKIT_MAJOR_VERSION,
         core::STORMKIT_MINOR_VERSION,
         core::STORMKIT_PATCH_VERSION,
         core::STORMKIT_GIT_BRANCH,
         core::STORMKIT_GIT_COMMIT_HASH);

    curl_global_init(CURL_GLOBAL_ALL);
    parseSettings();
    loadPlugins();

    m_bot = std::make_unique<dpp::cluster>(m_token);

    m_bot->on_log([](const auto &event) {
        switch (event.severity) {
            case dpp::ll_debug: DPP_LOGGER.dlog("{}", event.message); break;
            case dpp::ll_info: DPP_LOGGER.ilog("{}", event.message); break;
            case dpp::ll_warning: DPP_LOGGER.wlog("{}", event.message); break;
            case dpp::ll_error: DPP_LOGGER.elog("{}", event.message); break;
            case dpp::ll_critical: DPP_LOGGER.flog("{}", event.message); break;
            default: break;
        }
    });

    m_bot->on_ready([this](const auto &event) {
        ilog("logged as \"{}\"", m_bot->me.username);

        initializeBot();

        for (auto &plugin : m_plugins) { plugin.interface->onReady(event, *m_bot); }
    });
}

/////////////////////////////////////
/////////////////////////////////////
Inquisitor::~Inquisitor() {
    for (auto &plugin : m_plugins) {
        auto res = plugin.loader.func<void(PluginInterface *)>("deallocatePlugin");
        if (res) {
            auto &&deallocate_func = res.value();
            deallocate_func(plugin.interface);
        } else
            elog("Failed to load plugin {} deallocator, reason: {}",
                 plugin.interface->name(),
                 res.error().message());
    }

    curl_global_cleanup();
}

/////////////////////////////////////
/////////////////////////////////////
auto Inquisitor::run([[maybe_unused]] const core::Int32 argc, [[maybe_unused]] const char **argv)
    -> core::Int32 {
    m_bot->start(false);

    return EXIT_SUCCESS;
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
    for (auto &enabled_plugin : enabled_plugins) {
        m_enabled_plugins.emplace_back(enabled_plugin);

        auto options = json {};

        if (document.contains(enabled_plugin)) options = document[enabled_plugin];

        options["inquisitor"] = json::parse(
            std::format(R"({{ "major": {}, "minor": {} }})", MAJOR_VERSION, MINOR_VERSION));

        m_plugin_options.emplace(enabled_plugin, std::move(options));
    }
}

/////////////////////////////////////
/////////////////////////////////////
auto Inquisitor::loadPlugins() -> void {
    for (auto &plugin : std::filesystem::recursive_directory_iterator("plugins")) {
        const auto plugin_path = plugin.path();

        if (plugin_path.extension() == ".so" || plugin_path.extension() == ".dll") {
            ilog("{} found", plugin_path.string());
            loadPlugin(plugin_path);
        }
    }
}

/////////////////////////////////////
/////////////////////////////////////
auto Inquisitor::loadPlugin(const std::filesystem::path &path) -> void {
    auto res = core::DynamicLoader::load(path);
    if (!res) {
        elog("Failed to load plugin {}", res.error().message());
        return;
    }

    auto &&loader = res.value();
    auto res2     = loader.func<PluginInterface *()>("allocatePlugin");
    if (!res2) {
        elog("Failed to initialize plugin {}", res.error().message());
        return;
    }

    auto &&allocate_func  = res2.value();
    auto plugin_interface = allocate_func();

    const auto plugin_name = plugin_interface->name();

    if (std::ranges::find(m_enabled_plugins, plugin_interface->name()) ==
        std::ranges::cend(m_enabled_plugins))
        return;

    ilog("{} loaded", plugin_name);

    m_plugins.emplace_back(Plugin { std::move(path), std::move(loader), plugin_interface });
}

/////////////////////////////////////
/////////////////////////////////////
auto Inquisitor::initializeBot() -> void {
    struct Commands {
        PluginInterface *plugin;
        std::vector<PluginInterface::Command> commands;
    };

    auto plugins = std::vector<Commands> {};

    auto _plugins = std::vector<const PluginInterface *> {};
    for (const auto &plugin : m_plugins) _plugins.emplace_back(plugin.interface);

    for (auto &plugin : m_plugins) {
        plugin.interface->initialize(m_plugin_options.at(std::string { plugin.interface->name() }),
                                     _plugins);

        if (!std::empty(plugin.interface->commands())) {
            auto plugin_command     = Commands {};
            plugin_command.plugin   = plugin.interface;
            plugin_command.commands = plugin.interface->commands();
            for (auto command_ : plugin.interface->commands()) {
                auto command = dpp::slashcommand {}
                                   .set_name(std::string { command_.name })
                                   .set_description(std::string { command_.description })
                                   .set_application_id(m_bot->me.id)
                                   .set_type(dpp::ctxm_chat_input);

                m_bot->global_command_create(command);
            }
            plugins.emplace_back(std::move(plugin_command));
        }
    }

    m_bot->on_interaction_create([this, plugins = std::move(plugins)](const auto &event) {
        auto cmd_data = std::get<dpp::command_interaction>(event.command.data);

        for (auto &plugin : plugins)
            for (auto &command : plugin.commands)
                if (cmd_data.name == command.name) plugin.plugin->onCommand(event, *m_bot);
    });

    m_bot->on_message_create([this](const auto &event) {
        for (auto &plugin : m_plugins) { plugin.interface->onMessageReceived(event, *m_bot); }
    });
    /*


        m_asio_context = std::make_shared<boost::asio::io_context>();
        m_bot = std::make_shared<Bot>();
        m_bot->intents = discordpp::intents::GUILD_MESSAGES |
       discordpp::intents::DIRECT_MESSAGE_REACTIONS | discordpp::intents::GUILDS;
        m_bot->debugUnhandled = false;
        m_bot->prefix = ";inquisitor ";

        m_bot->handlers.emplace(std::string{"READY"}, std::function<void(json)>{[this](json data){
                                    ilog("Connected !");
                                    for(auto &plugin : m_plugins) {
                                        plugin.interface->onReady(data);
                                    }
                                }});

        m_bot->handlers.emplace(std::string{"MESSAGE_CREATE"}, std::function<void(json)>{[this](json
       data){ for(auto &plugin : m_plugins) { plugin.interface->onMessageReceived(data);
                                    }
                                }});

        auto plugins = std::vector<const PluginInterface *>{};
        for(const auto &plugin :m_plugins)
            plugins.emplace_back(plugin.interface);

        const auto send_message = [this](std::string_view channel_id,
                                         const json &msg) {
            m_bot->callJson()
                ->method("POST")
                ->target(std::format("/channels/{}/messages", channel_id))
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
                ->target(std::format("/channels/{}/messages", channel_id))
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
                ->target(std::format("/channels/{}/messages/{}", channel_id, message_id))
                ->onRead(
                    [on_response = std::move(on_response), channel_id, message_id](const bool error,
       const json msg) { if(error) { elog("Failed to get message {} on channel {}, retrying",
       message_id, channel_id); return;
                        }
                        on_response(msg);
                    })->run();
        };

        const auto get_channel = [this](std::string_view channel_id,
                                        std::function<void(const json &)> on_response) {
            m_bot->call()
                ->method("GET")
                ->target(std::format("/channels/{}", channel_id))
                ->onRead([on_response = std::move(on_response), channel_id](const bool error, const
       json msg) { if(error) { elog("Failed to get channel {}, retrying", channel_id); return;
                    }

                    on_response(msg);
                })->run();

        };

        const auto get_all_message = [this](std::string_view channel_id, std::function<void(const
       json &)> on_response) { m_bot->call()
                ->method("GET")
                ->target(std::format("/channels/{}/messages", channel_id))
                ->onRead([on_response = std::move(on_response), channel_id](const bool error, const
       json msg) { if(error) { elog("Failed to get all messages from channel {}", channel_id);
                        return;
                    }

                    on_response(msg);
                })->run();
        };

        const auto delete_message = [this](std::string_view channel_id, std::string_view message_id)
       { auto payload = json{};

            m_bot->callJson()
                ->method("DELETE")
                ->target(std::format("/channels/{}/messages/{}", channel_id, message_id))
                ->payload(std::move(payload))
                ->onRead([channel_id, message_id](const bool error, [[maybe_unused]] const json msg)
       { if(error) elog("Failed to get delete message {} from channel {}, retrying", message_id,
       channel_id);
                })->run();
        };

        const auto delete_messages = [this](std::string_view channel_id, std::span<const
       std::string> message_ids) { auto payload = json { { "messages", message_ids }
            };

            m_bot->callJson()
                ->method("POST")
                ->target(std::format("/channels/{}/messages/bulk-delete", channel_id))
                ->payload(std::move(payload))
                ->onRead([channel_id](const bool error, [[maybe_unused]] const json msg) {
                    if(error)
                        elog("Failed to get delete all messages from channel {}, retrying",
       channel_id);
                })->run();
        };

        const auto add_reaction = [this](std::string_view channel_id, std::string_view message_id,
       std::string_view emoji) { auto payload = json{};

            m_bot->callJson()
                ->method("PUT")
                ->target(std::format("/channels/{}/messages/{}/reactions/{}/@me", channel_id,
       message_id, emoji))
                ->payload(std::move(payload))
                ->onRead([channel_id, message_id](const bool error, [[maybe_unused]] const json msg)
       { if(error) elog("Failed to add reaction messages to message {} on channel {}, retrying",
       message_id, channel_id);
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
                                         },m_plugin_options.at(std::string{plugin.interface->name()}),
       plugins);

            for(auto command : plugin.interface->commands()) {
                m_bot->respond(std::string{command},
                               [command, &plugin](json data) {
                                   plugin.interface->onCommand(command, data);
                               });
            }
        }

        m_bot->initBot(9, "Bot " + m_token, m_asio_context);*/
}

#pragma pop_macro("interface")
