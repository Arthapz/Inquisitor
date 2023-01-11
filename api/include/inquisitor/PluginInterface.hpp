// Copryright (C) 2023 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

#pragma once

#include <inquisitor/CoreDependencies.hpp>

class PluginInterface {
  public:
    using json = nlohmann::json;

    using SendMessageFunction = std::function<void(std::string_view, const json &)>;
    using SendFileFunction =
        std::function<void(std::string_view, std::string, std::string, std::string, const json &)>;
    using GetMessageFunction =
        std::function<void(std::string_view, std::string_view, std::function<void(const json &)>)>;
    using GetChannelFunction =
        std::function<void(std::string_view, std::function<void(const json &)>)>;
    using GetAllMessageFunction =
        std::function<void(std::string_view, std::function<void(const json &)>)>;
    using DeleteMessageFunction = std::function<void(std::string_view, std::string_view)>;
    using DeleteMessagesFunction =
        std::function<void(std::string_view, std::span<const std::string>)>;
    using AddReactionFunction =
        std::function<void(std::string_view, std::string_view, std::string_view)>;

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

    struct Command {
        std::string_view name;
        std::string_view description;
    };

    PluginInterface() noexcept;
    virtual ~PluginInterface() = 0;

    void initialize(const json &options, std::vector<const PluginInterface *> others);

    [[nodiscard]] virtual auto name() const -> const std::string      & = 0;
    [[nodiscard]] virtual auto commands() const -> std::vector<Command> = 0;
    virtual auto onCommand([[maybe_unused]] const dpp::interaction_create_t &,
                           [[maybe_unused]] dpp::cluster &) -> void {};

    virtual auto onReady([[maybe_unused]] const dpp::ready_t &, [[maybe_unused]] dpp::cluster &)
        -> void {};
    virtual auto onMessageReceived([[maybe_unused]] const dpp::message_create_t &,
                                   [[maybe_unused]] dpp::cluster &) -> void {}

  protected:
    virtual auto initialize([[maybe_unused]] const json &options) -> void {};

    SendMessageFunction sendMessage;
    SendFileFunction sendFile;
    GetMessageFunction getMessage;
    GetChannelFunction getChannel;
    GetAllMessageFunction getAllMessage;
    DeleteMessageFunction deleteMessage;
    DeleteMessagesFunction deleteMessages;
    AddReactionFunction addReaction;

    GetHttpFile getHttpFile;

    std::vector<const PluginInterface *> m_others;
};

#define INQUISITOR_PLUGIN(type)                                                \
    extern "C" {                                                               \
    STORMKIT_EXPORT [[nodiscard]] auto allocatePlugin() -> type *;             \
    STORMKIT_EXPORT [[nodiscard]] auto deallocatePlugin(type *plugin) -> void; \
    }                                                                          \
    auto allocatePlugin()->type * { return new type {}; }                      \
    auto deallocatePlugin(type *plugin)->void { delete plugin; }
