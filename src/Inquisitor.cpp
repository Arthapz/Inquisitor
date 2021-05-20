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
    m_hello_channel_ids = document["hello_channel_ids"].get<std::vector<std::string>>();

    auto enabled_plugins = document["enabled_plugins"].get<std::vector<std::string>>();
    for(auto &enabled_plugin : enabled_plugins) {
        m_enabled_plugins.emplace_back(enabled_plugin);

        if(document.contains(enabled_plugin))
            m_plugin_options.emplace(enabled_plugin, document[enabled_plugin].dump());
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

    auto it = std::ranges::find_if(m_plugin_options, [&plugin_name](const auto &p){ return p.first == plugin_name; });
    if(it != std::ranges::cend(m_plugin_options))
        plugin_interface->setOptions(it->second);

    plugin_interface->initialize([this](std::string channel_id, const json &msg) {
        m_bot->call("POST",
                    fmt::format("/channels/{}/messages", channel_id),
                    msg);
    });

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
                                onReady();
                            }});

    m_bot->handlers.emplace(std::string{"MESSAGE_CREATE"}, std::function<void(json)>{[this](json data){
                                for(auto &plugin : m_plugins) {
                                    plugin.interface->onMessageReceived(data);
                                }
                            }});

    m_bot->respond("help", [this](json msg){ printHelp(msg); });
    m_bot->respond("plugins", [this](json msg){ printPlugins(msg); });

    for(auto &plugin : m_plugins) {
        if(!std::empty(plugin.interface->command())) {
            m_bot->respond(std::string{plugin.interface->command()},
                           [&plugin](json data) { plugin.interface->onCommand(data); });
        }
    }

    m_bot->initBot(9, "Bot " + m_token, m_asio_context);
}

/////////////////////////////////////
/////////////////////////////////////
auto Inquisitor::onReady() -> void {
    ilog("Connected !");

    auto response = json{};
    response["content"] = fmt::format("-- 🤖 Inquisitor V{}.{} initialized 🤖 --", Inquisitor::MAJOR_VERSION, Inquisitor::MINOR_VERSION);
    response["tts"] = false;

    for(const auto &id : m_hello_channel_ids) {
        m_bot->call("POST",
                    fmt::format("/channels/{}/messages", id),
                    std::move(response));
    }
}

/////////////////////////////////////
/////////////////////////////////////
auto Inquisitor::printHelp(const json &msg) -> void {
    const auto result = std::time(nullptr);

    auto help_string = std::string{};

    const auto title       = fmt::format("Inquisitor {}.{} commands",
                                   Inquisitor::MAJOR_VERSION,
                                   Inquisitor::MINOR_VERSION);
    static constexpr auto DESCRIPTION_FORMAT = "🔵 **{}** -> {} \n";

    auto description = std::string{ "🔵 **help** -> Print this message \n"
        "🔵 **plugins** -> Print loaded plugins\n"};
    for(auto &p : m_plugins) {
        auto &plugin = p.interface;
        if(plugin->command() == "") continue;

        description += fmt::format("\n------- {} -------\n", plugin->name());
        description += fmt::format(DESCRIPTION_FORMAT, plugin->command(), plugin->help());
    }

    auto footer =
        fmt::format("Requested by {} at {}", msg["author"]["username"].get<std::string>(), std::asctime(std::localtime(&result)));
    footer.erase(std::remove(std::begin(footer), std::end(footer), '\n'), std::end(footer));

    auto response = json{
        {"content", ""},
        {"tts", false},
        {"embed", {
            { "title", title},
            { "type", "rich"},
            { "description", description},
            { "footer", {}}
        }}
    };
    response["footer"]["text"] = footer;

    const auto id = msg["channel_id"].get<std::string>();

    m_bot->call("POST",
        fmt::format("/channels/{}/messages", id),
                std::move(response));
}

auto Inquisitor::printPlugins(const json &msg) -> void {
    const auto &username = msg["author"]["username"].get<std::string>();

    const auto result = std::time(nullptr);

    const auto title       = fmt::format("Inquisitor {}.{} plugins",
                                   Inquisitor::MAJOR_VERSION,
                                   Inquisitor::MINOR_VERSION);
    static constexpr auto PLUGIN_FORMAT = "🔵 **{}** \n";

    auto description = std::string{};
    for(auto &p : m_plugins) {
        auto &plugin = p.interface;
        description += fmt::format(PLUGIN_FORMAT, plugin->name());
    }

    auto footer =
        fmt::format("Requested by {} at {}", username, std::asctime(std::localtime(&result)));
    footer.erase(std::remove(std::begin(footer), std::end(footer), '\n'), std::end(footer));

    auto response = json{
        {"content", ""},
        {"tts", false},
        {"embed", {
                       { "title", title},
                       { "type", "rich"},
                       { "description", description},
                       { "footer", {}}
                   }}
    };
    response["footer"]["text"] = footer;

    const auto id = msg["channel_id"].get<std::string>();

    m_bot->call("POST",
                fmt::format("/channels/{}/messages", id),
                std::move(response));
}
