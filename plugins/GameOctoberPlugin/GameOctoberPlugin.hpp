// Copryright (C) 2019 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

#pragma once

/////////// - STL - ///////////
#include <string>
#include <regex>

/////////// - Inquisitor-API - ///////////
#include <PluginInterface.hpp>

class GameOctoberPlugin final: public PluginInterface {
  public:
    GameOctoberPlugin() noexcept;
    ~GameOctoberPlugin() override;

    [[nodiscard]] std::string_view name() const override;
    [[nodiscard]] std::vector<Command> commands() const override;

    void onMessageReceived(const dpp::message_create_t &, dpp::cluster &) override;
  protected:
    void initialize(const json &options) override;

  private:
    std::regex m_regex;

    struct Guild {
        dpp::snowflake gallery;
        dpp::snowflake discussions;
    };

    std::vector<Guild> m_guilds;
};
