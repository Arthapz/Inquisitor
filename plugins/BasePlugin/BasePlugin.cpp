// Copryright (C) 2019 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

/////////// - BasePlugin - ///////////
#include "BasePlugin.hpp"
#include "Log.hpp"

/////////// - StormKit::core - ///////////
#include <storm/core/Strings.hpp>

INQUISITOR_PLUGIN(BasePlugin)

/////////////////////////////////////
/////////////////////////////////////
BasePlugin::BasePlugin() noexcept = default;

/////////////////////////////////////
/////////////////////////////////////
BasePlugin::~BasePlugin() = default;

/////////////////////////////////////
/////////////////////////////////////
auto BasePlugin::name() const -> std::string_view {
    return "BasePlugin";
}

/////////////////////////////////////
/////////////////////////////////////
auto BasePlugin::commands() const -> std::vector<std::string_view> {
    return {"help", "plugins", "about"};
}

/////////////////////////////////////
/////////////////////////////////////
auto BasePlugin::help() const -> std::string_view {
    return "🔵 **help** -> Print this message\n🔵 **plugins** -> Show plugins\n🔵 **about** -> Show about message";
}

/////////////////////////////////////
/////////////////////////////////////
auto BasePlugin::onReady(const json &msg) -> void {
    auto response = json{};
    response["content"] = fmt::format("-- 🤖 Inquisitor V{}.{} initialized 🤖 --", m_major_version, m_minor_version);
    response["tts"] = false;

    for(const auto &id : m_channels) {
        sendMessage(id, response);
    }
}

/////////////////////////////////////
/////////////////////////////////////
auto BasePlugin::onCommand(std::string_view command, const json &msg) -> void {
    if(command == "help")
        sendHelp(msg);
    else if(command == "plugins")
        sendPlugins(msg);
    else if(command == "about")
        sendAbout(msg);
}

/////////////////////////////////////
/////////////////////////////////////
auto BasePlugin::initialize(const json &options) -> void {
    m_major_version = options["inquisitor"]["major"].get<storm::core::UInt32>();
    m_minor_version = options["inquisitor"]["minor"].get<storm::core::UInt32>();

    m_channels = options["channels"].get<std::vector<std::string>>();

    static constexpr auto PLUGIN_FORMAT = "🔵 **{}** \n";

    m_help_string = "";
    for(const auto &plugin_ptr : m_others) {
        if(!std::empty(plugin_ptr->commands())) {
        m_help_string += fmt::format("\n\n------- {} -------\n", plugin_ptr->name());
        m_help_string += plugin_ptr->help();
        }

        m_plugins_string += fmt::format(PLUGIN_FORMAT, plugin_ptr->name());
    }

    m_about_string = fmt::format("**author**: Arthapz\n**organization**: Tapzcrew\n**sources**: https://gitlab.com/tapzcrew/inquisitor-cpp", m_major_version, m_minor_version);
}

/////////////////////////////////////
/////////////////////////////////////
auto BasePlugin::sendHelp(const json &msg) -> void {
    const auto result = std::time(nullptr);

    const auto title       = fmt::format("Inquisitor {}.{} commands",
                                   m_major_version,
                                   m_minor_version);

    auto footer =
        fmt::format("Requested by {} at {}", msg["author"]["username"].get<std::string>(), std::asctime(std::localtime(&result)));
    footer.erase(std::remove(std::begin(footer), std::end(footer), '\n'), std::end(footer));

    auto response = json{
        {"content", ""},
        {"tts", false},
        {"embed", {
                       { "title", title},
                       { "type", "rich"},
                       { "description", m_help_string},
                       { "footer", {}}
                   }}
    };
    response["footer"]["text"] = footer;

    sendMessage(msg["channel_id"].get<std::string>(), response);
}

/////////////////////////////////////
/////////////////////////////////////
auto BasePlugin::sendPlugins(const json &msg) -> void {
    const auto result = std::time(nullptr);

    const auto title       = fmt::format("Inquisitor {}.{} commands",
                                   m_major_version,
                                   m_minor_version);

    auto footer =
        fmt::format("Requested by {} at {}", msg["author"]["username"].get<std::string>(), std::asctime(std::localtime(&result)));
    footer.erase(std::remove(std::begin(footer), std::end(footer), '\n'), std::end(footer));

    auto response = json{
        {"content", ""},
        {"tts", false},
        {"embed", {
                       { "title", title},
                       { "type", "rich"},
                       { "description", m_plugins_string},
                       { "footer", {}}
                   }}
    };
    response["footer"]["text"] = footer;
    sendMessage(msg["channel_id"].get<std::string>(), response);
}

/////////////////////////////////////
/////////////////////////////////////
auto BasePlugin::sendAbout(const json &msg) -> void {
    const auto result = std::time(nullptr);

    const auto title       = fmt::format("Inquisitor version {}.{}",
                                   m_major_version,
                                   m_minor_version);

    auto footer =
        fmt::format("Requested by {} at {}", msg["author"]["username"].get<std::string>(), std::asctime(std::localtime(&result)));
    footer.erase(std::remove(std::begin(footer), std::end(footer), '\n'), std::end(footer));

    auto response = json{
        {"content", ""},
        {"tts", false},
        {"embed", {
                       { "title", title},
                       { "type", "rich"},
                       { "description", m_about_string},
                       { "footer", {}}
                   }}
    };
    response["footer"]["text"] = footer;
    sendMessage(msg["channel_id"].get<std::string>(), response);
}
