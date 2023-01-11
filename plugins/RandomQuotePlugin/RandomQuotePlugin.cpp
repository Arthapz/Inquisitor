// Copryright (C) 2019 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

/////////// - RandomQuotePlugin - ///////////
#include "RandomQuotePlugin.hpp"
#include "Log.hpp"

/////////// - STL - ///////////
#include <iostream>
#include <chrono>

/////////// - StormKit::core - ///////////
#include <storm/core/Strings.hpp>

INQUISITOR_PLUGIN(RandomQuotePlugin)

using namespace std::literals;

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
auto RandomQuotePlugin::name() const -> std::string_view {
    return "RandomQuotePlugin";
}

/////////////////////////////////////
/////////////////////////////////////
auto RandomQuotePlugin::commands() const -> std::vector<Command> {
    return {
       Command{ "random-quote", "Print a random quote" },
       Command{ "rq", "Shortcut for random-quote" },
    };
}

/////////////////////////////////////
/////////////////////////////////////
auto RandomQuotePlugin::onCommand(const dpp::interaction_create_t &event, [[maybe_unused]] dpp::cluster &bot) -> void {
    auto quote = getQuote();

    if(!std::empty(quote))
        event.reply(dpp::ir_channel_message_with_source, dpp::message{quote});
}

/////////////////////////////////////
/////////////////////////////////////
auto RandomQuotePlugin::onMessageReceived(const dpp::message_create_t &event, dpp::cluster &bot) -> void {
    auto it = std::ranges::find_if(m_last_sended_messages, [&event](const auto &p) { return p.first == std::to_string(event.msg->channel_id); });
    if(it == std::ranges::cend(m_last_sended_messages)) return;

    auto now = Clock::now();

    auto &tp = m_last_sended_messages[it->first];

    if(std::chrono::duration_cast<std::chrono::seconds>(now - tp) < 10min) return;

    tp = now;

    auto n = m_send_distribution(m_generator);
    ilog("{}", n);
    if(n >= 50 && n <= 60) {
        auto quote = getQuote();

        if(!std::empty(quote))
            bot.message_create(dpp::message{event.msg->channel_id, quote});
    }
}


/////////////////////////////////////
/////////////////////////////////////
auto RandomQuotePlugin::initialize(const json &options) -> void {
    if(!options.contains("channels")) {
        elog("Missing channels array in options");
        return;
    }

    if(!options["channels"].is_array()) {
        elog("Option entry \"channels\" array need to be an array");
        return;
    }

    m_channels = options["channels"].get<std::vector<std::string>>();

    for(const auto &channel : m_channels) {
        ilog("{}", channel);
        m_last_sended_messages[channel] = Clock::now();
    }
}

/////////////////////////////////////
/////////////////////////////////////
auto RandomQuotePlugin::getQuote() -> std::string {
    if(std::empty(m_quote_list)) return "";

    auto quote_n = m_quote_distribution(m_generator);

    return m_quote_list[quote_n];
}
