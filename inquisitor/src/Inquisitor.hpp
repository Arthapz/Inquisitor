// Copryright (C) 2023 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

#pragma once

#include "CoreDependencies.hpp"

class Inquisitor final: public stormkit::core::App {
  public:
    using json = nlohmann::json;

    static constexpr auto MAJOR_VERSION = 4;
    static constexpr auto MINOR_VERSION = 0;
    static constexpr auto PATCH_VERSION = 0;

    Inquisitor() noexcept;
    ~Inquisitor() override;

    auto run(const stormkit::core::Int32 argc, const char **argv) -> stormkit::core::Int32 override;

    auto stop() noexcept -> void;

  private:
    auto parseSettings() -> void;
    auto loadPlugins() -> void;
    auto loadPlugin(const std::filesystem::path &path) -> void;
    auto initializeBot() -> void;

    std::atomic_bool m_run = true;

    std::string m_token;
    std::string m_hello_channel_id;

    struct Plugin {
        std::filesystem::path path;
        stormkit::core::DynamicLoader loader;
        PluginInterface *interface;
    };

    std::vector<std::string> m_enabled_plugins;
    std::vector<Plugin> m_plugins;

    stormkit::core::HashMap<std::string, json> m_plugin_options;

    std::unique_ptr<dpp::cluster> m_bot;
};
