// Copryright (C) 2019 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

#pragma once

/////////// - STL - ///////////
#include <string>
#include <regex>

/////////// - Inquisitor-API - ///////////
#include <PluginInterface.hpp>

class MelonPlugin final: public PluginInterface {
  public:
    MelonPlugin() noexcept;
    ~MelonPlugin() override;

    [[nodiscard]] std::string_view name() const override;
    [[nodiscard]] std::vector<Command> commands() const override;

    void onMessageReceived(const dpp::message_create_t &, dpp::cluster &) override;
  private:
    std::regex m_regex;
};
