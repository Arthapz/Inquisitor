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
auto PluginInterface::initialize(Functions &&functions,  const json &options, std::vector<const PluginInterface*> others) -> void {
    sendMessage = std::move(functions.send_func);
    getMessage = std::move(functions.get_message_func);
    getChannel = std::move(functions.get_channel_func);

    m_others = std::move(others);

    initialize(options);
}
