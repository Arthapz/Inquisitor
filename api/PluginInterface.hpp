// Copryright (C) 2019 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

#pragma once

/////////// - STL - ///////////
#include <string>
#include <string_view>

/////////// - nlohmann-json - ///////////
#include <nlohmann/json.hpp>

class PluginInterface;

class PluginInterface {
  public:
    using json = nlohmann::json;

    using SendFunction = std::function<void(std::string channel_id, const json &msg)>;

    PluginInterface() noexcept;
    virtual ~PluginInterface() = 0;

    void initialize(SendFunction &&func, const json &options, std::vector<const PluginInterface*> others);

    [[nodiscard]] virtual std::string_view name() const    = 0;
    [[nodiscard]] virtual std::vector<std::string_view> commands() const = 0;
    [[nodiscard]] virtual std::string_view help() const    = 0;
    virtual void onCommand(std::string_view command, const json &msg) {};

    virtual void onReady(const json &msg) {};
    virtual void onMessageReceived(const json& msg) {};

  protected:
    virtual void initialize(const json &options) {};

    SendFunction sendMessage;
    std::vector<const PluginInterface*> m_others;
};

#define INQUISITOR_PLUGIN(type)                    \
    extern "C" {                                   \
    type *allocatePlugin();                        \
    void deallocatePlugin(type *plugin);           \
    }                                              \
    type *allocatePlugin() { return new type {}; } \
    void deallocatePlugin(type *plugin) { delete plugin; }
