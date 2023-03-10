// Copryright (C) 2023 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

#include <inquisitor/PluginInterface.hpp>

/////////////////////////////////////
/////////////////////////////////////
PluginInterface::PluginInterface() noexcept = default;

/////////////////////////////////////
/////////////////////////////////////
PluginInterface::~PluginInterface() = default;

/////////////////////////////////////
/////////////////////////////////////
auto PluginInterface::initialize(const json &options, std::vector<const PluginInterface*> others) -> void {
    /*
    sendMessage = std::move(functions.send_message_func);
    sendFile = std::move(functions.send_file_func);
    getMessage = std::move(functions.get_message_func);
    getChannel = std::move(functions.get_channel_func);
    getAllMessage = std::move(functions.get_all_message_func);
    deleteMessage = std::move(functions.delete_message_func);
    deleteMessages = std::move(functions.delete_messages_func);
    addReaction = std::move(functions.add_reaction_func);

    getHttpFile = std::move(functions.get_http_file_func);*/

    m_others = std::move(others);

    initialize(options);
}
