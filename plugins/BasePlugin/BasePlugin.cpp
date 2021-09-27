// Copryright (C) 2019 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

/////////// - BasePlugin - ///////////
#include "BasePlugin.hpp"
#undef FMT_HEADER_ONLY
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
auto BasePlugin::commands() const -> std::vector<Command> {
    return {
       Command{"help", "Print this message"},
       Command{"plugins", "Show loaded plugins"},
       Command{"about", "Show about message"},
    };
}

/////////////////////////////////////
/////////////////////////////////////
auto BasePlugin::onReady([[maybe_unused]] const dpp::ready_t &event, dpp::cluster &bot) -> void {
    const auto str =  storm::core::format("-- :robot: Inquisitor V{}.{} initialized :robot: --", m_major_version, m_minor_version);

    for(const auto &id : m_channels) {
        auto id_as_snowflake = static_cast<dpp::snowflake>(std::stoll(id));
        bot.message_create(dpp::message{id_as_snowflake, str});
    }
}

/////////////////////////////////////
/////////////////////////////////////
auto BasePlugin::onCommand(const dpp::interaction_create_t &event, [[maybe_unused]] dpp::cluster &bot) -> void {
    auto cmd_data = std::get<dpp::command_interaction>(event.command.data);

    if(cmd_data.name == "help")
        sendHelp(event);
    else if(cmd_data.name == "plugins")
        sendPlugins(event);
    else if(cmd_data.name == "about")
        sendAbout(event);
}

/////////////////////////////////////
/////////////////////////////////////
auto BasePlugin::initialize(const json& options) -> void {
    m_major_version = options["inquisitor"]["major"].get<storm::core::UInt32>();
    m_minor_version = options["inquisitor"]["minor"].get<storm::core::UInt32>();

    m_channels = options["channels"].get<std::vector<std::string>>();

    static constexpr auto PLUGIN_FORMAT = "🔵 **{}** \n";

    m_help_string = "";
    for(const auto &plugin_ptr : m_others) {
        if(!std::empty(plugin_ptr->commands())) {
            m_help_string += storm::core::format("\n\n------- 🔵 **{}** -------\n", plugin_ptr->name());

            for(const auto command : plugin_ptr->commands())
                m_help_string += storm::core::format("{} -> {}\n", command.name, command.description);
        }

        m_plugins_string += storm::core::format(PLUGIN_FORMAT, plugin_ptr->name());
    }

    m_about_string = storm::core::format("**author**: Arthapz\n**organization**: Tapzcrew\n**sources**: https://gitlab.com/tapzcrew/inquisitor-cpp", m_major_version, m_minor_version);
}

/////////////////////////////////////
/////////////////////////////////////
auto BasePlugin::sendHelp(const dpp::interaction_create_t& event) -> void {
    auto title = storm::core::format("Inquisitor {}.{} commands",
                             m_major_version,
                             m_minor_version);

    auto embed = dpp::embed{}
                     .set_title(std::move(title))
                     .set_description(m_help_string);

    auto message = dpp::message{}
                       .add_embed(std::move(embed));

    event.reply(dpp::ir_channel_message_with_source, std::move(message));
}

/////////////////////////////////////
/////////////////////////////////////
auto BasePlugin::sendPlugins(const dpp::interaction_create_t& event) -> void {
    auto title       = storm::core::format("Inquisitor {}.{} loaded plugins",
                                         m_major_version,
                                         m_minor_version);

    auto embed = dpp::embed{}
                     .set_title(std::move(title))
                    // .set_type('rich')
                     .set_description(m_plugins_string);

    auto message = dpp::message{}
                       .add_embed(std::move(embed));

    event.reply(dpp::ir_channel_message_with_source, std::move(message));
}

/////////////////////////////////////
/////////////////////////////////////
auto BasePlugin::sendAbout(const dpp::interaction_create_t &event) -> void {
    auto title       = storm::core::format("Inquisitor version {}.{}",
                                         m_major_version,
                                         m_minor_version);

    auto embed = dpp::embed{}
                     .set_title(std::move(title))
                    // .set_type('rich')
                     .set_description(m_about_string);

    auto message = dpp::message{}
                       .add_embed(std::move(embed));

    event.reply(dpp::ir_channel_message_with_source, std::move(message));
}
