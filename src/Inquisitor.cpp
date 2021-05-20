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
    discordpp::log::filter = discordpp::log::info;
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

    for(auto &plugin : m_plugins) {
    plugin.interface->initialize([this](std::string channel_id, const json &msg) {
        m_bot->call("POST",
                    fmt::format("/channels/{}/messages", channel_id),
                    msg);
        }, m_plugin_options.at(std::string{plugin.interface->name()}), plugins);

        for(auto command : plugin.interface->commands()) {
            m_bot->respond(std::string{command},
                               [command, &plugin](json data) { plugin.interface->onCommand(command, data); });
        }
    }

    m_bot->initBot(9, "Bot " + m_token, m_asio_context);
}
