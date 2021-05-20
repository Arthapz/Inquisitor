// Copryright (C) 2019 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

/////////// - QuoteMessagePlugin - ///////////
#include "QuoteMessagePlugin.hpp"

INQUISITOR_PLUGIN(QuoteMessagePlugin)

/////////////////////////////////////
/////////////////////////////////////
QuoteMessagePlugin::QuoteMessagePlugin() = default;

/////////////////////////////////////
/////////////////////////////////////
QuoteMessagePlugin::~QuoteMessagePlugin() = default;

/////////////////////////////////////
/////////////////////////////////////
auto QuoteMessagePlugin::name() const -> std::string_view {
    return "QuoteMessagePlugin";
}

/////////////////////////////////////
/////////////////////////////////////
auto QuoteMessagePlugin::commands() const -> std::vector<std::string_view> {
    return {};
}

/////////////////////////////////////
/////////////////////////////////////
auto QuoteMessagePlugin::help() const -> std::string_view {
    return "";
}

/////////////////////////////////////
/////////////////////////////////////
auto QuoteMessagePlugin::onMessageReceived(const json &msg) -> void {

}
