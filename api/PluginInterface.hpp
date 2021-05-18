// Copryright (C) 2019 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

#pragma once

/////////// - STL - ///////////
#include <string_view>

class PluginInterface {
  public:
    PluginInterface();
    virtual ~PluginInterface() = 0;

    [[nodiscard]] virtual std::string_view name() const    = 0;
    [[nodiscard]] virtual std::string_view command() const = 0;
    [[nodiscard]] virtual std::string_view help() const    = 0;

    [[nodiscard]] virtual void run() const {}
};

#define INQUISITOR_PLUGIN(type)                    \
    extern "C" {                                   \
    type *allocatePlugin();                        \
    void deallocatePlugin(type *plugin);           \
    }                                              \
    type *allocatePlugin() { return new type {}; } \
    void deallocatePlugin(type *plugin) { delete plugin; }
