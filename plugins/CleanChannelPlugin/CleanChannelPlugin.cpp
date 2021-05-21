// Copryright (C) 2019 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

/////////// - CleanChannelPlugin - ///////////
#include "CleanChannelPlugin.hpp"
#include "Log.hpp"

#include <ranges>
#include <chrono>
#include <charconv>

INQUISITOR_PLUGIN(CleanChannelPlugin)

/////////////////////////////////////
/////////////////////////////////////
CleanChannelPlugin::CleanChannelPlugin() noexcept = default;

/////////////////////////////////////
/////////////////////////////////////
CleanChannelPlugin::~CleanChannelPlugin() = default;

/////////////////////////////////////
/////////////////////////////////////
auto CleanChannelPlugin::name() const -> std::string_view {
    return "CleanChannelPlugin";
}

/////////////////////////////////////
/////////////////////////////////////
auto CleanChannelPlugin::commands() const -> std::vector<std::string_view> {
    return {"clean-channel"};
}

/////////////////////////////////////
/////////////////////////////////////
auto CleanChannelPlugin::help() const -> std::string_view {
    return "🔵 **clean-channel** -> Clean (remove all non-images posts) a channel, only usable by moderators";
}

/////////////////////////////////////
/////////////////////////////////////
auto CleanChannelPlugin::onCommand(std::string_view command, const json &msg) -> void {
    auto moderator = msg["author"]["id"].get<std::string>();
    auto channel = msg["channel_id"].get<std::string>();

    auto is_legal_user = std::ranges::find(m_moderators, moderator) != std::ranges::cend(m_moderators);
    if(!is_legal_user) {
        auto response = json {
            {"content", "You are not a moderator, abording"}
        };

        sendMessage(channel, std::move(response));

        return;
    }

    auto is_legal_channel = std::ranges::find(m_channels, channel) != std::ranges::cend(m_channels);
    if(!is_legal_channel) {
        auto response = json {
            {"content", "Invalid channel, abording"}
        };

        sendMessage(channel, std::move(response));

        return;
    }

    {
        auto response = json {
            {"content", "Cleaning channel ..."}
        };
        sendMessage(channel, std::move(response));
    }
    cleanChannel(std::move(channel));
}

/////////////////////////////////////
/////////////////////////////////////
auto CleanChannelPlugin::initialize(const json &options) -> void {
    if(!options.contains("channels")) {
        elog("Missing channels array in options");
        return;
    }

    if(!options["channels"].is_array()) {
        elog("Option entry \"channels\" array need to be an array");
        return;
    }

    m_channels = options["channels"].get<std::vector<std::string>>();

    if(!options.contains("moderators")) {
        elog("Missing moderators array in options");
        return;
    }

    if(!options["moderators"].is_array()) {
        elog("Option entry \"moderators\" array need to be an array");
        return;
    }

    m_moderators = options["moderators"].get<std::vector<std::string>>();
}

/////////////////////////////////////
/////////////////////////////////////
auto CleanChannelPlugin::cleanChannel(std::string channel) -> void {
    static constexpr auto NO_IMAGE = [](const json &message) {
        return std::empty(message["attachments"]);
    };

    static constexpr auto GET_MESSAGE_ID = [](const json &message) {
        return message["id"].get<std::string>();
    };

    static constexpr auto UNDER_14 = [](const json &message) {
        auto now = std::chrono::high_resolution_clock::now();

        auto delta = now - std::chrono::days{14};

        auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(delta.time_since_epoch()).count();

        auto message_timestamp_string = storm::core::split(message["timestamp"].get<std::string>(), '.');
        auto ss = std::istringstream{message_timestamp_string[0]};

        auto tm = std::tm{};
        ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");

        auto tp = std::chrono::high_resolution_clock::from_time_t(std::mktime(&tm)).time_since_epoch().count() / 1000000000;

        return timestamp <= tp;
    };

    getAllMessage(channel, [this, channel = channel](const json &msgs) {
        auto messages = std::vector<json>{};
        std::ranges::for_each(msgs["body"], [&messages](const auto &json) { messages.emplace_back(json); });

        auto ids_view = messages | std::views::filter(UNDER_14) | std::views::filter(NO_IMAGE) | std::views::transform(GET_MESSAGE_ID);
        auto ids = std::vector<std::string>{std::ranges::begin(ids_view), std::ranges::end(ids_view)};

        ilog("{}", std::size(ids));

        if(std::empty(ids)) return;
        else if(std::size(ids) == 1)
            deleteMessage(channel, ids[0]);
        else deleteMessages(channel, ids);

        cleanChannel(std::move(channel));
    });
}
