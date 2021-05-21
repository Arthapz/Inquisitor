// Copryright (C) 2019 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

#pragma once

/////////// - STL - ///////////
#include <string>
#include <regex>

/////////// - Inquisitor-API - ///////////
#include <PluginInterface.hpp>

class QuoteMessagePlugin final: public PluginInterface {
  public:
    QuoteMessagePlugin() noexcept;
    ~QuoteMessagePlugin() override;

    [[nodiscard]] std::string_view name() const override;
    [[nodiscard]] std::vector<std::string_view> commands() const override;
    [[nodiscard]] std::string_view help() const override;

    void onMessageReceived(const json &msg) override;
  private:
    std::regex m_regex;
};
