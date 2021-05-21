// Copryright (C) 2019 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

#pragma once

/////////// - STL - ///////////
#include <string>
#include <regex>

/////////// - Inquisitor-API - ///////////
#include <PluginInterface.hpp>

class CleanChannelPlugin final: public PluginInterface {
  public:
    CleanChannelPlugin() noexcept;
    ~CleanChannelPlugin() override;

    [[nodiscard]] std::string_view name() const override;
    [[nodiscard]] std::vector<std::string_view> commands() const override;
    [[nodiscard]] std::string_view help() const override;

    void onCommand(std::string_view command, const json &msg) override;

  protected:
    void initialize(const json &options) override;

  private:
    void cleanChannel(std::string channel);

    std::vector<std::string> m_moderators;

    std::vector<std::string> m_channels;
};
