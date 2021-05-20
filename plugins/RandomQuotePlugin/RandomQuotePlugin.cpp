// Copryright (C) 2019 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

/////////// - RandomQuotePlugin - ///////////
#include "RandomQuotePlugin.hpp"
#include "Log.hpp"

/////////// - STL - ///////////
#include <iostream>

/////////// - StormKit::core - ///////////
#include <storm/core/Strings.hpp>

INQUISITOR_PLUGIN(RandomQuotePlugin)

/////////////////////////////////////
/////////////////////////////////////
RandomQuotePlugin::RandomQuotePlugin()
    : m_generator{std::random_device{}()}, m_send_distribution{0, 100}, m_quote_distribution{0, 1} {
    auto stream = std::ifstream{};
    stream.open("facts.txt");

    if(!stream) {
        elog("Failed to open facts.txt");
        return;
    }

    const auto string =
        std::string { std::istreambuf_iterator<char> { stream }, std::istreambuf_iterator<char> {} };

    m_quote_list = storm::core::split(string, '\n');

    m_quote_distribution = std::uniform_int_distribution<storm::core::UInt32>{0, gsl::narrow_cast<storm::core::UInt32>(std::size(m_quote_list))};
}

/////////////////////////////////////
/////////////////////////////////////
RandomQuotePlugin::~RandomQuotePlugin() = default;

/////////////////////////////////////
/////////////////////////////////////
auto RandomQuotePlugin::setOptions(std::string_view options) -> void {
    auto object = json::parse(options);

    if(!object.contains("channels")) {
        elog("Missing channels array in options");
        return;
    }

    if(!object["channels"].is_array()) {
        elog("Option entry \"channels\" array need to be an array");
        return;
    }

    m_channels = object["channels"].get<std::vector<std::string>>();

    for(const auto &channel : m_channels)
        m_last_sended_messages[channel] = Clock::now();
}

/////////////////////////////////////
/////////////////////////////////////
auto RandomQuotePlugin::name() const -> std::string_view {
    return "RandomQuotePlugin";
}

/////////////////////////////////////
/////////////////////////////////////
auto RandomQuotePlugin::command() const -> std::string_view {
    return "random-quote";
}

/////////////////////////////////////
/////////////////////////////////////
auto RandomQuotePlugin::help() const -> std::string_view {
    return "Print a random quote";
}

/////////////////////////////////////
/////////////////////////////////////
void RandomQuotePlugin::onCommand(const json &msg) {
    sendQuote(msg);
}

/////////////////////////////////////
/////////////////////////////////////
auto RandomQuotePlugin::onMessageReceived(const json &msg) -> void {
    auto it = std::ranges::find_if(m_last_sended_messages, [&msg](const auto &p) { return p.first == msg["channel_id"].get<std::string>(); });
    if(it == std::ranges::cend(m_last_sended_messages)) return;

    auto now = Clock::now();

    auto &tp = m_last_sended_messages[it->first];

    if(std::chrono::duration_cast<std::chrono::milliseconds>(now - tp).count() < 6000) return;

    tp = now;

    auto n = m_send_distribution(m_generator);
    if(n == 50) sendQuote(msg);
}

/////////////////////////////////////
/////////////////////////////////////
void RandomQuotePlugin::sendQuote(const json &msg) {
    if(std::empty(m_quote_list)) return;

    auto quote_n = m_quote_distribution(m_generator);

    auto quote = std::string_view{m_quote_list[quote_n]};

    auto response = json{};
    response["content"] = quote;
    response["tts"] = false;
    sendMessage(msg["channel_id"].get<std::string>(), response);
}
