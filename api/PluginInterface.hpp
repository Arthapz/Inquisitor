// Copryright (C) 2019 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

#pragma once

/////////// - STL - ///////////
#include <string>
#include <string_view>
#include <future>

/////////// - nlohmann-json - ///////////
#include <nlohmann/json.hpp>

class PluginInterface;

class PluginInterface {
  public:
    using json = nlohmann::json;

    using SendFunction = std::function<void(std::string channel_id, const json &msg)>;
    using GetMessageFunction = std::function<void(std::string channel_id, std::string message_id, std::function<void(const json &json)> on_response)>;
    using GetChannelFunction = std::function<void(std::string channel_id, std::function<void(const json &json)> on_response)>;

    struct Functions {
        SendFunction send_func;
        GetMessageFunction get_message_func;
        GetChannelFunction get_channel_func;
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

    SendFunction sendMessage;
    GetMessageFunction getMessage;
    GetChannelFunction getChannel;

    std::vector<const PluginInterface*> m_others;
};

#define INQUISITOR_PLUGIN(type)                    \
    extern "C" {                                   \
    type *allocatePlugin();                        \
    void deallocatePlugin(type *plugin);           \
    }                                              \
    type *allocatePlugin() { return new type {}; } \
    void deallocatePlugin(type *plugin) { delete plugin; }
