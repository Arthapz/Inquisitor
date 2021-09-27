// Copryright (C) 2019 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

/////////// - QuoteMessagePlugin - ///////////
#include "QuoteMessagePlugin.hpp"
#undef FMT_HEADER_ONLY
#include "Log.hpp"

INQUISITOR_PLUGIN(QuoteMessagePlugin)

static constexpr auto REGEX = R"(https?:\/\/discord.com\/channels\/([[:digit:]]+)\/([[:digit:]]+)\/([[:digit:]]+))";

/////////////////////////////////////
/////////////////////////////////////
QuoteMessagePlugin::QuoteMessagePlugin() noexcept
    : m_regex{REGEX, std::regex::ECMAScript | std::regex::optimize | std::regex::icase} {}

/////////////////////////////////////
/////////////////////////////////////
QuoteMessagePlugin::~QuoteMessagePlugin() = default;

/////////////////////////////////////
/////////////////////////////////////
auto QuoteMessagePlugin::name() const -> std::string_view {
    return "QuoteMessagePlugin";
}

/////////////////////////////////////
/////////////////////////////////////
auto QuoteMessagePlugin::commands() const -> std::vector<Command> {
    return {};
}

/////////////////////////////////////
/////////////////////////////////////
auto QuoteMessagePlugin::onMessageReceived(const dpp::message_create_t &event, dpp::cluster &bot) -> void {
    auto matches = std::smatch{};
    const auto &content = event.msg->content;

    if(!std::regex_search(content, matches, m_regex)) return;

    const auto guild_id = event.msg->guild_id;

    const auto channel_id = static_cast<dpp::snowflake>(std::stoll(matches[2].str()));
    const auto message_id = static_cast<dpp::snowflake>(std::stoll(matches[3].str()));

    const auto target_channel_id = event.msg->channel_id;

    bot.message_get(message_id, channel_id, [&bot, guild_id, content, target_channel_id](const auto &event){
        if(event.is_error())
            elog("{}", event.get_error().message);
        else {
            const auto &message = std::get<dpp::message>(event.value);

            if(message.guild_id != guild_id) return;

            const auto name = (std::empty(message.member.nickname)) ?
                  message.author->username : message.member.nickname;

            auto author = dpp::embed_author{
                .name = name,
                .url  = storm::core::format("https://discordapp.com/users/{}", message.author->id),
                .icon_url = message.author->get_avatar_url()
            };

            auto embed = dpp::embed{}
                .set_description(message.content)
                .set_author(std::move(author));

            auto reply = dpp::message{target_channel_id, std::move(embed)};

            bot.message_create(reply);
        }
    });
}
