// Copryright (C) 2019 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

/////////// - Inquisitor - ///////////
#include "Bot.hpp"
#include "Inquisitor.hpp"
#include "Log.hpp"

/////////// - SL - ///////////
#include <ctime>

/////////// - StormKit::core - ///////////
#include <storm/core/Strings.hpp>

static constexpr auto INQUISITOR_HELP = "Inquisitor {}.{} commands:\n\t help > print this message";

/////////////////////////////////////
/////////////////////////////////////
Bot::~Bot() = default;

/////////////////////////////////////
/////////////////////////////////////
auto Bot::onReady(SleepyDiscord::Ready readyData) -> void {
    sendMessage(m_hello_channel_id, fmt::format("-- 🤖 Inquisitor V{}.{} initialized 🤖 --", Inquisitor::MAJOR_VERSION, Inquisitor::MINOR_VERSION));
}

/////////////////////////////////////
/////////////////////////////////////
auto Bot::onMessage(SleepyDiscord::Message message) -> void {
    const auto command = storm::core::split(message.content, ' ');
    if (std::empty(command) || command[0] != ";inquisitor") return;

    if (command[1] == "help") printHelp(message);
    else if (command[1] == "plugins")
        printPlugins(message);
    else {
        for (const auto &plugin : m_plugins) {
            if (command[2] == plugin->command()) {}
        }
    }
}

void Bot::tick(float delta_time) {
    for(auto &plugin : m_plugins) {
        plugin->run();
    }
}

/////////////////////////////////////
/////////////////////////////////////
auto Bot::addPlugin(PluginInterface *plugin) -> void {
    m_plugins.emplace_back(plugin);
}

void Bot::setChannelID(std::string id) {
    m_hello_channel_id = std::move(id);
}

/////////////////////////////////////
/////////////////////////////////////
auto Bot::printHelp(const SleepyDiscord::Message &message) -> void {
    const auto &username = message.author.username;

    const auto result = std::time(nullptr);

    static constexpr auto EMBED_STRING = R"(
    {{
        "title": "{}",
        "type": "{}",
        "description": "{}",
        "author": {{
            "name": "{}",
            "iconUrl": "{}"
        }},
        "footer": {{
            "text": "{}"
        }}
    }})";

    const auto title       = fmt::format("Inquisitor {}.{} commands",
                                   Inquisitor::MAJOR_VERSION,
                                   Inquisitor::MINOR_VERSION);
    const auto description = "🔵 **help** ➡️ print this message \\n"
                             "🔵 **plugins** ➡️ print loaded plugins";
    auto footer =
        fmt::format("Requested by {} at {}", username, std::asctime(std::localtime(&result)));
    footer.erase(std::remove(std::begin(footer), std::end(footer), '\n'), std::end(footer));

    const auto embed_string = fmt::format(EMBED_STRING,
                                          title,
                                          "rich",
                                          description,
                                          username,
                                          message.author.avatar,
                                          footer);

    auto embed = SleepyDiscord::Embed { embed_string };
    sendMessage(message.channelID, "", embed);
}

/////////////////////////////////////
/////////////////////////////////////
auto Bot::printPlugins(const SleepyDiscord::Message &message) -> void {
    const auto &username = message.author.username;

    const auto result = std::time(nullptr);

    static constexpr auto EMBED_STRING = R"(
    {{
        "title": "{}",
        "type": "{}",
        "description": "{}",
        "author": {{
            "name": "{}",
            "iconUrl": "{}"
        }},
        "footer": {{
            "text": "{}"
        }}
    }})";

    const auto title       = fmt::format("Inquisitor {}.{} commands",
                                   Inquisitor::MAJOR_VERSION,
                                   Inquisitor::MINOR_VERSION);
    static constexpr auto PLUGIN_FORMAT = "🔵 {} \\n";

    auto description = std::string{};
    for(auto &plugin : m_plugins)
        description += fmt::format(PLUGIN_FORMAT, plugin->name());

    auto footer =
        fmt::format("Requested by {} at {}", username, std::asctime(std::localtime(&result)));
    footer.erase(std::remove(std::begin(footer), std::end(footer), '\n'), std::end(footer));

    const auto embed_string = fmt::format(EMBED_STRING,
                                          title,
                                          "rich",
                                          description,
                                          username,
                                          message.author.avatar,
                                          footer);

    auto embed = SleepyDiscord::Embed { embed_string };
    sendMessage(message.channelID, "", embed);
}
