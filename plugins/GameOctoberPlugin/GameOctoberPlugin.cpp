// Copryright (C) 2019 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

/////////// - GameOctoberPlugin - ///////////
#include "GameOctoberPlugin.hpp"
#undef FMT_HEADER_ONLY
#include "Log.hpp"

#if __cpp_lib_chrono < 201907L
    #include <ctime>
#endif

INQUISITOR_PLUGIN(GameOctoberPlugin)

using namespace std::literals;

static constexpr auto REGEX = R"(https?:\/\/(www\.)?[-a-zA-Z0-9@:%._\+~#=]{1,256}\.[a-zA-Z0-9()]{1,6}\b([-a-zA-Z0-9()@:%_\+.~#?&//=]*))";

static constexpr auto THEMES = std::array {
    "Cristal"sv,
    "Costume"sv,
    "Navire"sv,
    "Noeud"sv,
    "Corbeau"sv,
    "Esprit"sv,
    "Ventilateur"sv,
    "Montre"sv,
    "Pression"sv,
    "Choix / Pioche"
    "Acide / Aigre"sv,
    "Collé"sv,
    "Toiture"sv,
    "Cocher"sv,
    "Casque"sv,
    "Boussole"sv,
    "Percuter"sv,
    "Lune"sv,
    "Boucle / Circuit"sv,
    "Germer"sv,
    "Flou / Vague"sv,
    "Ouvert"sv,
    "Fuir"sv,
    "Disparu"sv,
    "Eclaboussure"sv,
    "Relier"sv,
    "Etincelle"sv,
    "Crousillant"sv,
    "Pièce"sv,
    "Glisser"sv,
    "Risque / Danger"sv,
};

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
auto GameOctoberPlugin::onReady(const dpp::ready_t &event, dpp::cluster &bot) -> void {
    m_timer.makeTask(1min, 1min, [this, &bot](){
        if(m_current_word < std::size(THEMES)) {
                bot.message_create(dpp::message {
                        891960563649896449ull,
                        storm::core::format("A vos claviers ! Le thème du jour est \"{}\".", THEMES[m_current_word++])
                    },
                    [](const auto &event){
                        if(event.is_error()) elog("{}", event.http_info.body);
                    }
                );
        } else m_current_word = 0;
    });
}

/////////////////////////////////////
/////////////////////////////////////
auto GameOctoberPlugin::onMessageReceived(const dpp::message_create_t &event, dpp::cluster &bot) -> void {
    const auto &message = *event.msg;
    if(message.author->id == bot.me.id) return;

    auto it = std::ranges::find_if(m_guilds, [channel_id = event.msg->channel_id](const auto &guild){ return guild.gallery == channel_id; });

    if(it == std::ranges::end(m_guilds)) return;

    auto matches = std::smatch{};
    const auto has_url = std::regex_search(message.content, matches, m_regex);

    if(std::empty(message.attachments) && !has_url) {
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
#else
    std::time_t tt = std::chrono::system_clock::to_time_t(now);
    std::tm utc_tm = *gmtime(&tt);
    std::tm local_tm = *localtime(&tt);

    auto d = utc_tm.tm_mday;
#endif

    bot.thread_create_with_message(
        storm::core::format("{}-{}-{}-gameoctober", THEMES[m_current_word], name, d),
        message.channel_id,
        message.id,
        1440,
        [](const auto &event) {
            if(event.is_error()) elog("{}", event.http_info.body);
    });
}
