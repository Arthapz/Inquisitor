// Copryright (C) 2021 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

#pragma once

/////////// - STL - ///////////
#include <atomic>
#include <filesystem>
#include <string>
#include <vector>

/////////// - StormKit::core - ///////////
#include <storm/core/App.hpp>

/////////// - StormKit::module - ///////////
#include <storm/module/Module.hpp>

/////////// - Inquisitor - ///////////
#include "Fwd.hpp"

/////////// - Inquisitor-api - ///////////
#include <PluginInterface.hpp>

class Inquisitor final: public storm::core::App {
  public:
    static constexpr auto MAJOR_VERSION = "3";
    static constexpr auto MINOR_VERSION = "0";

    Inquisitor() noexcept;
    ~Inquisitor() override;

    void run(const int argc, const char **argv) override;

    void stop() noexcept;

  private:
    void parseSettings();
    void loadPlugins();
    void loadPlugin(const std::filesystem::path &path);

    std::atomic_bool m_run = true;

    std::string m_token;
    std::string m_hello_channel_id;

    BotOwnedPtr m_bot;

    struct Plugin {
        std::filesystem::path path;
        storm::module::Module module;
        PluginInterface *interface;
    };

    std::vector<Plugin> m_plugins;
};
