// Copryright (C) 2019 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

#pragma once

/////////// - STL - ///////////
#include <string>
#include <random>

/////////// - Inquisitor-API - ///////////
#include <PluginInterface.hpp>

class RandomQuotePlugin final: public PluginInterface {
  public:
    RandomQuotePlugin();
    ~RandomQuotePlugin() override;

    [[nodiscard]] std::string_view name() const override;
    [[nodiscard]] std::string_view command() const override;
    [[nodiscard]] std::string_view help() const override;

    [[nodiscard]] void run() const override;

    void run() const override;

  private:
    std::uniform_int_distribution<storm::core::UInt32> m_distribution;

    std::vector<std::string> m_quote_list;
};
