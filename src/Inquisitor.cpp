// Copryright (C) 2021 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

/////////// - STL - ///////////
#include <csignal>

/////////// - Inquisitor - ///////////
#include "Inquisitor.hpp"
#include "Log.hpp"

/////////////////////////////////////
/////////////////////////////////////
Inquisitor::Inquisitor() noexcept {
    ilog("Using StormKit {}.{}.{} {} {}",
         STORMKIT_MAJOR_VERSION,
         STORMKIT_MINOR_VERSION,
         STORMKIT_PATCH_VERSION,
         STORMKIT_GIT_BRANCH,
         STORMKIT_GIT_COMMIT_HASH);

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
    m_bot->intents = discordpp::intents::GUILD_MESSAGES;
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
        m_bot->call("POST",
                    fmt::format("/channels/{}/messages", channel_id),
                    msg,
                    [channel_id](const bool error, [[maybe_unused]] const json msg) {
                        if(error)
                            elog("Failed to send message to channel {}, retrying", channel_id);
                    });
    };

    const auto get_message = [this](std::string_view channel_id,
                                    std::string_view message_id,
                                    std::function<void(const json &)> on_response) {
        m_bot->call("GET",
                    fmt::format("/channels/{}/messages/{}", channel_id, message_id),
                    [on_response = std::move(on_response), channel_id, message_id](const bool error, const json msg) {
                        if(error) {
                            elog("Failed to get message {} on channel {}, retrying", message_id, channel_id);
                            return;
                        }
                        on_response(msg);
                    });
    };

    const auto get_channel = [this](std::string_view channel_id,
                                    std::function<void(const json &)> on_response) {
        m_bot->call("GET",
                    fmt::format("/channels/{}", channel_id),
                    [on_response = std::move(on_response), channel_id](const bool error, const json msg) {
                        if(error) {
                            elog("Failed to get channel {}, retrying", channel_id);
                            return;
                        }

                        on_response(msg);
                    });

    };

    const auto get_all_message = [this](std::string_view channel_id, std::function<void(const json &)> on_response) {
        //auto payload = json{
        //    { "limit", 100 }
        //};
        //ilog("{}", payload.dump());
        m_bot->call("GET",
                    fmt::format("/channels/{}/messages", channel_id),
                    //std::move(payload),
                    [on_response = std::move(on_response), channel_id](const bool error, const json msg) {
                        if(error) {
                            elog("Failed to get all messages from channel {}", channel_id);
                            return;
                        }

                        on_response(msg);
                    });
    };

    const auto delete_message = [this](std::string_view channel_id, std::string_view message_id) {
        m_bot->call("DELETE",
                    fmt::format("/channels/{}/messages/{}", channel_id, message_id),
                    [channel_id, message_id](const bool error, [[maybe_unused]] const json msg) {
                        if(error)
                            elog("Failed to get delete message {} from channel {}, retrying", message_id, channel_id);
                    });
    };

    const auto delete_messages = [this](std::string_view channel_id, std::span<const std::string> message_ids) {
        auto payload = json {
            { "messages", message_ids }
        };

        m_bot->call("POST",
                    fmt::format("/channels/{}/messages/bulk-delete", channel_id),
                    std::move(payload),
                    [channel_id](const bool error, [[maybe_unused]] const json msg) {
                        if(error)
                            elog("Failed to get delete all messages from channel {}, retrying", channel_id);
                    });
    };

    for(auto &plugin : m_plugins) {
        plugin.interface->initialize(PluginInterface::Functions {send_message,
                                                                 get_message,
                                                                 get_channel,
                                                                 get_all_message,
                                                                 delete_message,
                                                                 delete_messages
        },m_plugin_options.at(std::string{plugin.interface->name()}), plugins);

        for(auto command : plugin.interface->commands()) {
            m_bot->respond(std::string{command},
                           [command, &plugin](json data) { plugin.interface->onCommand(command, data); });
        }
    }

    m_bot->initBot(9, "Bot " + m_token, m_asio_context);
}
