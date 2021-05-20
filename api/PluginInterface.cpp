// Copryright (C) 2019 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

/////////// - Inquisitor-API - ///////////
#include "PluginInterface.hpp"

/////////////////////////////////////
/////////////////////////////////////
PluginInterface::PluginInterface() noexcept = default;

/////////////////////////////////////
/////////////////////////////////////
PluginInterface::~PluginInterface() = default;

/////////////////////////////////////
/////////////////////////////////////
auto PluginInterface::initialize(SendFunction &&func, const json &options, std::vector<const PluginInterface*> others) -> void {
    sendMessage = std::move(func);

    m_others = std::move(others);

    initialize(options);
}
