// Copryright (C) 2019 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

/////////// - GalleryPlugin - ///////////
#include "GalleryPlugin.hpp"
#undef FMT_HEADER_ONLY
#include "Log.hpp"

#include <ranges>
#include <chrono>
#include <charconv>

#if __cpp_lib_chrono < 201907L
    #include <ctime>
#endif

INQUISITOR_PLUGIN(GalleryPlugin)

static constexpr auto REGEX = R"(https?:\/\/(www\.)?[-a-zA-Z0-9@:%._\+~#=]{1,256}\.[a-zA-Z0-9()]{1,6}\b([-a-zA-Z0-9()@:%_\+.~#?&//=]*))";

/////////////////////////////////////
/////////////////////////////////////
GalleryPlugin::GalleryPlugin() noexcept
    : m_regex{REGEX, std::regex::ECMAScript | std::regex::optimize | std::regex::icase} {}

/////////////////////////////////////
/////////////////////////////////////
GalleryPlugin::~GalleryPlugin() = default;

/////////////////////////////////////
/////////////////////////////////////
auto GalleryPlugin::name() const -> std::string_view {
    return "GalleryPlugin";
}

/////////////////////////////////////
/////////////////////////////////////
auto GalleryPlugin::commands() const -> std::vector<Command> {
    return {};
}

/////////////////////////////////////
/////////////////////////////////////
auto GalleryPlugin::initialize(const json &options) -> void {
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
auto GalleryPlugin::onMessageReceived(const dpp::message_create_t &event, dpp::cluster &bot) -> void {
    auto it = std::ranges::find_if(m_channels, [channel_id = event.msg->channel_id](const auto &channel){ return channel == std::to_string(channel_id); });

    if(it == std::ranges::end(m_channels)) return;

    const auto &message = *event.msg;

    auto matches = std::smatch{};
    const auto has_url = std::regex_search(message.content, matches, m_regex);

    if(std::empty(message.attachments) && message.author->id != bot.me.id && !has_url) {
        bot.message_delete(message.id, message.channel_id, [](const auto &event){
                if(event.is_error()) elog("{}", event.http_info.body);
        });

        return;
    }

    const auto name = (std::empty(message.member.nickname)) ?
        message.author->username : message.member.nickname;

    auto now = std::chrono::system_clock::now();
#if __cpp_lib_chrono >= 201907L
    auto tp = std::chrono::zoned_time{std::chrono::current_zone(), now}.get_local_time();
    auto day = std::chrono::floor<std::chrono::days>(tp);
    auto ymd = std::chrono::year_month_day{day};

    auto d = std::uint32_t{ymd.day()};
    auto m = std::uint32_t{ymd.month()};
    auto y = std::int32_t{ymd.year()};
#else
    std::time_t tt = std::chrono::system_clock::to_time_t(now);
    std::tm utc_tm = *gmtime(&tt);
    std::tm local_tm = *localtime(&tt);

    auto d = utc_tm.tm_mday;
    auto m = utc_tm.tm_mon + 1 << '-';
    auto y = utc_tm.tm_year + 1900;
#endif

    bot.thread_create_with_message(
        std::format("galerie-{}-{}/{}/{}", name, d, m, y),
        message.channel_id,
        message.id,
        1440,
        [](const auto &event) {
            if(event.is_error()) elog("{}", event.http_info.body);
    });
}
