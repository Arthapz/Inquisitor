// Copryright (C) 2019 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

#pragma once

/////////// - STL - ///////////
#include <string>
#include <regex>
#include <chrono>
#include <atomic>

/////////// - StormKit::Core - ///////////
#include <storm/core/Timer.hpp>

/////////// - Inquisitor-API - ///////////
#include <PluginInterface.hpp>

class GameOctoberPlugin final: public PluginInterface {
  public:
    GameOctoberPlugin() noexcept;
    ~GameOctoberPlugin() override;

    [[nodiscard]] std::string_view name() const override;
    [[nodiscard]] std::vector<Command> commands() const override;

    void onReady(const dpp::ready_t &, dpp::cluster &) override;
    void onMessageReceived(const dpp::message_create_t &, dpp::cluster &) override;
  protected:
    void initialize(const json &options) override;

  private:
    std::regex m_regex;

    dpp::snowflake m_channel_id;

    std::atomic<std::size_t> m_current_word = 0;
    stormkit::core::Timer<std::chrono::high_resolution_clock, std::chrono::minutes> m_timer;

    std::atomic_bool m_started = false;
};
