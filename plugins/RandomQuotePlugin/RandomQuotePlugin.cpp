// Copryright (C) 2019 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

/////////// - RandomQuotePlugin - ///////////
#include "RandomQuotePlugin.hpp"

INQUISITOR_PLUGIN(RandomQuotePlugin)

/////////////////////////////////////
/////////////////////////////////////
RandomQuotePlugin::RandomQuotePlugin()
    : m_distribution()

/////////////////////////////////////
/////////////////////////////////////
RandomQuotePlugin::~RandomQuotePlugin() = default;

/////////////////////////////////////
/////////////////////////////////////
auto RandomQuotePlugin::name() const -> std::string_view {
    return "RandomQuotePlugin";
}

/////////////////////////////////////
/////////////////////////////////////
auto RandomQuotePlugin::command() const -> std::string_view {
    return "";
}

/////////////////////////////////////
/////////////////////////////////////
auto RandomQuotePlugin::help() const -> std::string_view {
    return "";
}

/////////////////////////////////////
/////////////////////////////////////
auto RandomQuotePlugin::run() const -> void {

}
