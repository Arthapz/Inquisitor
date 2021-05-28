// Copryright (C) 2019 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

/////////// - MelonPlugin - ///////////
#include "MelonPlugin.hpp"
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
auto MelonPlugin::commands() const -> std::vector<std::string_view> {
    return {};
}

/////////////////////////////////////
/////////////////////////////////////
auto MelonPlugin::help() const -> std::string_view {
    return "";
}

/////////////////////////////////////
/////////////////////////////////////
auto MelonPlugin::onMessageReceived(const json &msg) -> void {
    auto channel_id = msg["channel_id"].get<std::string>();
    auto message_id = msg["id"].get<std::string>();

    auto matches = std::smatch{};

    auto content = msg["content"].get<std::string>();

    if(!std::regex_search(content, matches, m_regex)) return;

    addReaction(channel_id, message_id, "%F0%9F%8D%88");
}
