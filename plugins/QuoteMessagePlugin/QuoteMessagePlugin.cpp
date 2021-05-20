// Copryright (C) 2019 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

/////////// - QuoteMessagePlugin - ///////////
#include "QuoteMessagePlugin.hpp"
#include "Log.hpp"

INQUISITOR_PLUGIN(QuoteMessagePlugin)

static constexpr auto REGEX = R"(https?:\/\/discord.com\/channels\/([[:digit:]]+)\/([[:digit:]]+)\/([[:digit:]]+))";

/////////////////////////////////////
/////////////////////////////////////
QuoteMessagePlugin::QuoteMessagePlugin()
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
auto QuoteMessagePlugin::commands() const -> std::vector<std::string_view> {
    return {};
}

/////////////////////////////////////
/////////////////////////////////////
auto QuoteMessagePlugin::help() const -> std::string_view {
    return "";
}

/////////////////////////////////////
/////////////////////////////////////
auto QuoteMessagePlugin::onMessageReceived(const json &msg) -> void {
    const auto result = std::time(nullptr);

    auto matches = std::smatch{};

    auto content = msg["content"].get<std::string>();

    if(!std::regex_search(content, matches, m_regex)) return;

    getChannel(matches[2].str(), [this, channel_id = matches[2].str(), message_id = matches[3].str(), msg, result](const json &get_response) {
        if(msg["guild_id"].get<std::string>() != get_response["body"]["guild_id"].get<std::string>()) return;

        getMessage(channel_id, message_id, [this, msg, result](const json &get_response){
            const auto &requested_msg = get_response["body"];

            auto footer =
                fmt::format("Quoted by {} at {}", msg["author"]["username"].get<std::string>(), std::asctime(std::localtime(&result)));
            footer.erase(std::remove(std::begin(footer), std::end(footer), '\n'), std::end(footer));

            auto avatar_hash = requested_msg["author"]["avatar"].get<std::string>();
            auto ext = (avatar_hash[0] == 'a' &&
                        avatar_hash[1] == '_') ? "gif" : "png";

            auto response = json{
                {"embed", {
                               {"author", {
                                       {"name", requested_msg["author"]["username"].get<std::string>()},
                                           {"icon_url", fmt::format("https://cdn.discordapp.com/avatars/{}/{}.{}", requested_msg["author"]["id"].get<std::string>(), avatar_hash, ext)}
                                   }
                               },
                               { "type", "rich"},
                               { "description", requested_msg["content"].get<std::string>()},
                               { "footer", {}}
                           }}
            };
            response["footer"]["text"] = footer;

            ilog("{}", response.dump());

            sendMessage(msg["channel_id"].get<std::string>(), response);
        });
    });
}
