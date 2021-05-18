// Copryright (C) 2021 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

#pragma once

/////////// - STL - ///////////
#include <string>
#include <vector>

/////////// - StormKit::core - ///////////
#include <storm/core/App.hpp>
#include <storm/core/Memory.hpp>

/////////// - Sleepy-Discord - ///////////
#include <sleepy_discord/sleepy_discord.h>

/////////// - Inquisitor - ///////////
#include "Fwd.hpp"

/////////// - Inquisitor-api - ///////////
#include <PluginInterface.hpp>

class Bot final: public SleepyDiscord::DiscordClient {
  public:
    using SleepyDiscord::DiscordClient::DiscordClient;
    ~Bot() override;

    void onReady(SleepyDiscord::Ready readyData) override;
    void onMessage(SleepyDiscord::Message message) override;

    void tick(float delta_time) override;

    void addPlugin(PluginInterface *plugin);

    void setChannelID(std::string id);

    ALLOCATE_HELPERS(Bot)
  private:
    void printHelp(const SleepyDiscord::Message &message);
    void printPlugins(const SleepyDiscord::Message &message);

    std::string m_token;
    std::string m_hello_channel_id;

    std::vector<PluginInterface *> m_plugins;
};
