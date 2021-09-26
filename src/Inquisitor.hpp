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
#include <storm/core/HashMap.hpp>

/////////// - StormKit::module - ///////////
#include <storm/module/Module.hpp>

/////////// - D++ - ///////////
#include <dpp/dpp.h>

/////////// - nlohmann-json - ///////////
#include <nlohmann/json.hpp>

/////////// - curl - ///////////
#include <curl/curl.h>

/////////// - Inquisitor - ///////////
#include "Fwd.hpp"

/////////// - Inquisitor-api - ///////////
#include <PluginInterface.hpp>

#pragma push_macro("interface")
#undef interface

class Inquisitor final: public storm::core::App {
  public:
    using json = nlohmann::json;

    static constexpr auto MAJOR_VERSION = 3;
    static constexpr auto MINOR_VERSION = 0;

    Inquisitor() noexcept;
    ~Inquisitor() override;

    void run(const int argc, const char **argv) override;

    void stop() noexcept;

  private:
    void parseSettings();
    void loadPlugins();
    void loadPlugin(const std::filesystem::path &path);
    void initializeBot();

    std::atomic_bool m_run = true;

    std::string m_token;
    std::string m_hello_channel_id;

    struct Plugin {
        std::filesystem::path path;
        storm::module::Module module;
        PluginInterface *interface;
    };

    std::vector<std::string> m_enabled_plugins;
    std::vector<Plugin> m_plugins;

    storm::core::HashMap<std::string, json> m_plugin_options;

    std::unique_ptr<dpp::cluster> m_bot;
};

#pragma pop_macro("interface")
