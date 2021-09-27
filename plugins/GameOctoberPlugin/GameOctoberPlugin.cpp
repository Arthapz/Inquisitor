// Copryright (C) 2019 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

/////////// - GameOctoberPlugin - ///////////
#include "GameOctoberPlugin.hpp"
#undef FMT_HEADER_ONLY
#include "Log.hpp"

INQUISITOR_PLUGIN(GameOctoberPlugin)

static constexpr auto REGEX = R"(https?:\/\/(www\.)?[-a-zA-Z0-9@:%._\+~#=]{1,256}\.[a-zA-Z0-9()]{1,6}\b([-a-zA-Z0-9()@:%_\+.~#?&//=]*))";

/////////////////////////////////////
/////////////////////////////////////
GameOctoberPlugin::GameOctoberPlugin() noexcept
    : m_regex{REGEX, std::regex::ECMAScript | std::regex::optimize | std::regex::icase} {}

/////////////////////////////////////
/////////////////////////////////////
GameOctoberPlugin::~GameOctoberPlugin() = default;

/////////////////////////////////////
/////////////////////////////////////
auto GameOctoberPlugin::name() const -> std::string_view {
    return "GameOctoberPlugin";
}

/////////////////////////////////////
/////////////////////////////////////
auto GameOctoberPlugin::commands() const -> std::vector<Command> {
    return {};
}

/////////////////////////////////////
/////////////////////////////////////
auto GameOctoberPlugin::initialize(const json &options) -> void {
    if(!options.contains("channels")) {
        elog("Missing channels array in options");
        return;
    }

    if(!options["channels"].is_array()) {
        elog("Option entry \"channels\" array need to be an array");
        return;
    }

    auto guilds = options["channels"].get<std::vector<json>>();


    for(auto _guild : guilds) {
        if(!_guild.is_object()) {
            elog("Option entry \"channels\" array need to be an array");
            return;
        }

        if(!_guild.contains("gallery") || !_guild.contains("discussions")) {
            elog("Option entry \"channels\" is ill-formed");
            return;
        }

        auto guild = Guild {
            static_cast<dpp::snowflake>(std::stoll(_guild["gallery"].get<std::string>())),
            static_cast<dpp::snowflake>(std::stoll(_guild["discussions"].get<std::string>()))
        };

        m_guilds.emplace_back(guild);
    }
}

/////////////////////////////////////
/////////////////////////////////////
auto GameOctoberPlugin::onMessageReceived(const dpp::message_create_t &event, dpp::cluster &bot) -> void {
    auto it = std::ranges::find_if(m_guilds, [channel_id = event.msg->channel_id](const auto &guild){ return guild.gallery == channel_id; });

    if(it == std::ranges::end(m_guilds)) return;

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
    auto tp = std::chrono::zoned_time{std::chrono::current_zone(), now}.get_local_time();
    auto day = std::chrono::floor<std::chrono::days>(tp);
    auto ymd = std::chrono::year_month_day{day};

    bot.thread_create_with_message(
        fmt::format("gameoctober-{}-{}", name, std::uint32_t{ymd.day()}),
        message.channel_id,
        message.id,
        1440,
        [](const auto &event) {
            if(event.is_error()) elog("{}", event.http_info.body);
    });

    /*const auto name = (std::empty(message.member.nickname)) ?
        message.author->username : message.member.nickname;

    auto author = dpp::embed_author{
        .name = name,
        .url  = fmt::format("https://discordapp.com/users/{}", message.author->id),
        .icon_url = message.author->get_avatar_url()
    };

    auto description = message.content;

    if(std::empty(message.attachments) && has_url) {
        const auto is_image = matches[0].str().find(".png") != std::string::npos ||
                              matches[0].str().find(".PNG") != std::string::npos ||
                              matches[0].str().find(".jpg") != std::string::npos ||
                              matches[0].str().find(".JPG") != std::string::npos ||
                              matches[0].str().find(".jpeg") != std::string::npos ||
                              matches[0].str().find(".JPEG") != std::string::npos ||
                              matches[0].str().find(".gif") != std::string::npos ||
                              matches[0].str().find(".GIF") != std::string::npos;

            auto embed = dpp::embed{}
               .set_author(std::move(author));

            if(is_image)
                embed.set_image(matches[0]);
            else
                embed.set_description(message.content);

            auto replica = dpp::message{it->discussions, std::move(embed)}
               .add_component(
                   dpp::component{}.add_component(dpp::component{}
                        .set_type(dpp::cot_button)
                        .set_label("Galerie")
                        .set_style(dpp::cos_link)
                        .set_url(fmt::format("https://discord.com/channels/{}/{}/{}", message.guild_id, message.channel_id, message.id))
                        .set_id("Galerie_button"))
               );
            bot.message_create(replica, [guild = *it, &bot](const auto &event){
                if(event.is_error()) elog("{}", event.http_info.body);
                else {
                    const auto &message = std::get<dpp::message>(event.value);
                    auto reply = dpp::message{guild.gallery, "Joli <:bave:626471505311563796>"}
                        .add_component(
                            dpp::component{}.add_component(dpp::component{}
                                .set_type(dpp::cot_button)
                                .set_label("Discussion")
                                .set_style(dpp::cos_link)
                                .set_url(fmt::format("https://discord.com/channels/{}/{}/{}", message.guild_id, message.channel_id, message.id))
                                .set_id("Discussion_button"))
                       );
                    bot.message_create(reply, [](const auto &event) {
                        if(event.is_error()) elog("{}", event.http_info.body);
                    });
                }
            });
    } else for(auto attachment : message.attachments) {
        if(attachment.content_type.find("image") != std::string::npos) {
            auto embed = dpp::embed{}
               .set_description(message.content)
               .set_author(std::move(author))
               .set_image(attachment.url);

            auto replica = dpp::message{it->discussions, std::move(embed)}
               .add_component(
                   dpp::component{}.add_component(dpp::component{}
                        .set_type(dpp::cot_button)
                        .set_label("Galerie")
                        .set_style(dpp::cos_link)
                        .set_url(fmt::format("https://discord.com/channels/{}/{}/{}", message.guild_id, message.channel_id, message.id))
                        .set_id("Galerie_button"))
               );
            bot.message_create(replica, [guild = *it, &bot](const auto &event){
                if(event.is_error()) elog("{}", event.http_info.body);
                else {
                    const auto &message = std::get<dpp::message>(event.value);
                    auto reply = dpp::message{guild.gallery, "Joli <:bave:626471505311563796>"}
                        .add_component(
                            dpp::component{}.add_component(dpp::component{}
                                .set_type(dpp::cot_button)
                                .set_label("Discussion")
                                .set_style(dpp::cos_link)
                                .set_url(fmt::format("https://discord.com/channels/{}/{}/{}", message.guild_id, message.channel_id, message.id))
                                .set_id("Discussion_button"))
                       );
                    bot.message_create(reply, [](const auto &event) {
                        if(event.is_error()) elog("{}", event.http_info.body);
                    });
                }
            });
        }

        description.clear();
    }*/

}
