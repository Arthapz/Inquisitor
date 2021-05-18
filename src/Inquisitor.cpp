// Copryright (C) 2021 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

/////////// - Inquisitor - ///////////
#include "Inquisitor.hpp"
#include "Bot.hpp"
#include "Log.hpp"

/////////// - RapidJSON - ///////////
#include <rapidjson/document.h>

/////////// - Sleepy-Discord - ///////////
#include <sleepy_discord/sleepy_discord.h>

namespace {
    Inquisitor *inquisitor;
}

/////////////////////////////////////
/////////////////////////////////////
auto sigHandler(int signal) noexcept -> void {
    if (signal == SIGQUIT || signal == SIGINT) inquisitor->stop();
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

    std::signal(SIGINT, sigHandler);
    std::signal(SIGQUIT, sigHandler);

    inquisitor = this;

    parseSettings();
    loadPlugins();
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

auto Inquisitor::stop() noexcept -> void {
    m_run = false;
}

/////////////////////////////////////
/////////////////////////////////////
auto Inquisitor::parseSettings() -> void {
    ilog("Loading settings.json");

    auto file = std::ifstream {};
    file.open("settings.json");

    const auto string =
        std::string { std::istreambuf_iterator<char> { file }, std::istreambuf_iterator<char> {} };

    auto document = rapidjson::Document {};
    document.Parse(std::data(string), std::size(string));

    m_token = document["token"].GetString();

    m_bot = Bot::allocateOwned(m_token);
    m_bot->setChannelID(document["hello_channel_id"].GetString());
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

auto Inquisitor::loadPlugin(const std::filesystem::path &path) -> void {
    auto module = storm::module::Module { path };

    if (!module.isLoaded()) elog("Failed to load plugin {}", path.string());

    auto allocate_func = module.getFunc<PluginInterface *()>("allocatePlugin");

    auto plugin_interface = allocate_func();

    ilog("{} loaded", plugin_interface->name());

    auto &plugin = m_plugins.emplace_back(Plugin { std::move(path), std::move(module), plugin_interface });

    m_bot->addPlugin(plugin.interface);
}
