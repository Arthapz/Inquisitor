// Copryright (C) 2019 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

/////////// - CleanChannelPlugin - ///////////
#include "CleanChannelPlugin.hpp"
#include "Log.hpp"

#include <ranges>

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
    return "🔵 **clean-channel** -> Clean (remove all non-images posts) a channel";
}

/////////////////////////////////////
/////////////////////////////////////
auto CleanChannelPlugin::onCommand(std::string_view command, const json &msg) -> void {
    auto channel = msg["channel_id"].get<std::string>();

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

    getAllMessage(channel, [this, channel = channel](const json &msgs) {
        auto messages = std::vector<json>{};
        std::ranges::for_each(msgs["body"], [&messages](const auto &json) { messages.emplace_back(json); });

        auto ids_view = messages | std::views::filter(NO_IMAGE) | std::views::transform(GET_MESSAGE_ID);
        auto ids = std::vector<std::string>{std::ranges::begin(ids_view), std::ranges::end(ids_view)};

        if(std::empty(ids)) return;
        else if(std::size(ids) == 1)
            deleteMessage(channel, ids[0]);
        else deleteMessages(channel, ids);

        cleanChannel(std::move(channel));
    });
}
