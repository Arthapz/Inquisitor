// Copryright (C) 2019 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

#pragma once

/////////// - STL - ///////////
#include <string>
#include <string_view>
#include <future>
#include <span>
#include <string_view>
#include <functional>

/////////// - nlohmann-json - ///////////
#include <nlohmann/json.hpp>

class PluginInterface;

class PluginInterface {
  public:
    using json = nlohmann::json;

    using SendMessageFunction = std::function<void(std::string_view, const json &)>;
    using SendFileFunction = std::function<void(std::string_view, std::string, std::string, std::string, const json &)>;
    using GetMessageFunction = std::function<void(std::string_view, std::string_view, std::function<void(const json &)>)>;
    using GetChannelFunction = std::function<void(std::string_view, std::function<void(const json &)>)>;
    using GetAllMessageFunction = std::function<void(std::string_view, std::function<void(const json &)>)>;
    using DeleteMessageFunction = std::function<void(std::string_view, std::string_view)>;
    using DeleteMessagesFunction = std::function<void(std::string_view, std::span<const std::string>)>;
    using AddReactionFunction = std::function<void(std::string_view, std::string_view, std::string_view)>;

    using GetHttpFile = std::function<std::string(std::string_view url)>;

    struct Functions {
        SendMessageFunction send_message_func;
        SendFileFunction send_file_func;
        GetMessageFunction get_message_func;
        GetChannelFunction get_channel_func;
        GetAllMessageFunction get_all_message_func;
        DeleteMessageFunction delete_message_func;
        DeleteMessagesFunction delete_messages_func;
        AddReactionFunction add_reaction_func;

        GetHttpFile get_http_file_func;
    };

    PluginInterface() noexcept;
    virtual ~PluginInterface() = 0;

    void initialize(Functions&& functions, const json &options, std::vector<const PluginInterface*> others);

    [[nodiscard]] virtual std::string_view name() const    = 0;
    [[nodiscard]] virtual std::vector<std::string_view> commands() const = 0;
    [[nodiscard]] virtual std::string_view help() const    = 0;
    virtual void onCommand(std::string_view command, const json &msg) {};

    virtual void onReady(const json &msg) {};
    virtual void onMessageReceived(const json& msg) {};

  protected:
    virtual void initialize(const json &options) {};

    SendMessageFunction sendMessage;
    SendFileFunction sendFile;
    GetMessageFunction getMessage;
    GetChannelFunction getChannel;
    GetAllMessageFunction getAllMessage;
    DeleteMessageFunction deleteMessage;
    DeleteMessagesFunction deleteMessages;
    AddReactionFunction addReaction;

    GetHttpFile getHttpFile;

    std::vector<const PluginInterface*> m_others;
};

#define INQUISITOR_PLUGIN(type)                    \
    extern "C" {                                   \
    type *allocatePlugin();                        \
    void deallocatePlugin(type *plugin);           \
    }                                              \
    type *allocatePlugin() { return new type {}; } \
    void deallocatePlugin(type *plugin) { delete plugin; }
