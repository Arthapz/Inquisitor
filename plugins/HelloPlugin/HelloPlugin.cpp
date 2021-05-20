// Copryright (C) 2019 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

/////////// - HelloPlugin - ///////////
#include "HelloPlugin.hpp"

INQUISITOR_PLUGIN(HelloPlugin)

/////////////////////////////////////
/////////////////////////////////////
HelloPlugin::HelloPlugin() = default;

/////////////////////////////////////
/////////////////////////////////////
HelloPlugin::~HelloPlugin() = default;

/////////////////////////////////////
/////////////////////////////////////
auto HelloPlugin::name() const -> std::string_view {
    return "HelloPlugin";
}

/////////////////////////////////////
/////////////////////////////////////
auto HelloPlugin::command() const -> std::string_view {
    return "hello";
}

/////////////////////////////////////
/////////////////////////////////////
auto HelloPlugin::help() const -> std::string_view {
    return "Say hello";
}
