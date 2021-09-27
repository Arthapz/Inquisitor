// Copryright (C) 2019 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

/////////// - MelonPlugin - ///////////
#include "MelonPlugin.hpp"
#undef FMT_HEADER_ONLY
#include "Log.hpp"

INQUISITOR_PLUGIN(MelonPlugin)

static constexpr auto REGEX = R"(melon)";

/////////////////////////////////////
/////////////////////////////////////
MelonPlugin::MelonPlugin() noexcept
    : m_regex{REGEX, std::regex::ECMAScript | std::regex::optimize | std::regex::icase} {}

/////////////////////////////////////
/////////////////////////////////////
MelonPlugin::~MelonPlugin() = default;

/////////////////////////////////////
/////////////////////////////////////
auto MelonPlugin::name() const -> std::string_view {
    return "MelonPlugin";
}

/////////////////////////////////////
/////////////////////////////////////
auto MelonPlugin::commands() const -> std::vector<Command> {
    return {};
}

/////////////////////////////////////
/////////////////////////////////////
auto MelonPlugin::onMessageReceived(const dpp::message_create_t &event, dpp::cluster &bot) -> void {
    auto matches = std::smatch{};
    const auto &content = event.msg->content;

    if(!std::regex_search(content, matches, m_regex)) return;

    bot.message_add_reaction(*event.msg, "🍈");
}
