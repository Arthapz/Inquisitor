// Copryright (C) 2019 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

#pragma once

/////////// - STL - ///////////
#include <string>

/////////// - Inquisitor-API - ///////////
#include <PluginInterface.hpp>

/////////// - StormKit::core - ///////////
#include <storm/core/Types.hpp>

class BasePlugin final: public PluginInterface {
  public:
    BasePlugin() noexcept;
    ~BasePlugin() override;

    [[nodiscard]] std::string_view name() const override;
    [[nodiscard]] std::vector<std::string_view> commands() const override;
    [[nodiscard]] std::string_view help() const override;

    void onReady(const json &msg) override;
    void onCommand(std::string_view command, const json &msg) override;

  protected:
    void initialize(const json &options) override;

  private:
    void sendHelp(const json &msg);
    void sendPlugins(const json &msg);
    void sendAbout(const json &msg);

    storm::core::UInt32 m_major_version;
    storm::core::UInt32 m_minor_version;

    std::vector<std::string> m_channels;

    std::string m_help_string;
    std::string m_plugins_string;
    std::string m_about_string;
};
