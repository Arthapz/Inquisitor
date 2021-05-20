// Copryright (C) 2019 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

#pragma once

/////////// - STL - ///////////
#include <string>
#include <string_view>
#include <random>
#include <chrono>

/////////// - Inquisitor-API - ///////////
#include <PluginInterface.hpp>

/////////// - StormKit::core - ///////////
#include <storm/core/Types.hpp>
#include <storm/core/HashMap.hpp>

class RandomQuotePlugin final: public PluginInterface {
  public:
    RandomQuotePlugin();
    ~RandomQuotePlugin() override;

    [[nodiscard]] std::string_view name() const override;
    [[nodiscard]] std::vector<std::string_view> commands() const override;
    [[nodiscard]] std::string_view help() const override;
    void onCommand(std::string_view command, const json &msg) override;

    void onMessageReceived(const json &msg) override;

  protected:
    void initialize(const json &options) override;

  private:
    using Clock = std::chrono::high_resolution_clock;

    void sendQuote(const json &msg);

    std::mt19937 m_generator;
    std::uniform_int_distribution<storm::core::UInt32> m_send_distribution;
    std::uniform_int_distribution<storm::core::UInt32> m_quote_distribution;

    std::vector<std::string> m_channels;

    std::vector<std::string> m_quote_list;

    storm::core::HashMap<std::string, Clock::time_point> m_last_sended_messages;
};
