// Copryright (C) 2021 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

#pragma once


/////////// - STL - ///////////
#include <atomic>
#include <filesystem>
#include <string>
#include <vector>

/////////// - StormKit::core - ///////////
#include <storm/core/App.hpp>
#include <storm/core/HashMap.hpp>

/////////// - StormKit::module - ///////////
#include <storm/module/Module.hpp>

/////////// - discordpp - ///////////
#include <discordpp/bot.hh>
#include <discordpp/log.hh>
#include <discordpp/plugin-overload.hh>
#include <discordpp/plugin-ratelimit.hh>
#include <discordpp/plugin-responder.hh>
#include <discordpp/rest-beast.hh>
#include <discordpp/websocket-simpleweb.hh>

template class discordpp::PluginResponder<discordpp::PluginOverload<discordpp::PluginRateLimit<discordpp::WebsocketSimpleWeb<discordpp::RestBeast<discordpp::Bot>>>>>;

/////////// - nlohmann-json - ///////////
#include <nlohmann/json.hpp>

/////////// - Boost - ///////////
#include <boost/asio.hpp>

/////////// - curl - ///////////
#include <curl/curl.h>

/////////// - Inquisitor - ///////////
#include "Fwd.hpp"

/////////// - Inquisitor-api - ///////////
#include <PluginInterface.hpp>

class Inquisitor final: public storm::core::App {
  public:
    using json = nlohmann::json;

    static constexpr auto MAJOR_VERSION = 3;
    static constexpr auto MINOR_VERSION = 0;

    Inquisitor() noexcept;
    ~Inquisitor() override;

    void run(const int argc, const char **argv) override;

    void stop() noexcept;

  private:
    using Bot = discordpp::PluginResponder<discordpp::PluginOverload<discordpp::PluginRateLimit<discordpp::WebsocketSimpleWeb<discordpp::RestBeast<discordpp::Bot>>>>>;

    void parseSettings();
    void loadPlugins();
    void loadPlugin(const std::filesystem::path &path);
    void initializeBot();

    std::atomic_bool m_run = true;

    std::string m_token;
    std::string m_hello_channel_id;

    struct Plugin {
        std::filesystem::path path;
        storm::module::Module module;
        PluginInterface *interface;
    };

    std::vector<std::string> m_enabled_plugins;
    std::vector<Plugin> m_plugins;

    storm::core::HashMap<std::string, json> m_plugin_options;

    std::shared_ptr<boost::asio::io_context> m_asio_context;
    std::shared_ptr<Bot> m_bot;
};
