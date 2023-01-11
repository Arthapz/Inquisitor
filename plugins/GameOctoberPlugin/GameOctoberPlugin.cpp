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
    "Choix / Pioche"sv,
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
    if(!options.contains("channel")) {
        elog("Missing channels array in options");
        return;
    }

    if(!options["channel"].is_string()) {
        elog("Option entry \"channels\" array need to be an array");
        return;
    }

    m_channel_id = static_cast<dpp::snowflake>(std::stoll(options["channel"].get<std::string>()));
}

/////////////////////////////////////
/////////////////////////////////////
auto GameOctoberPlugin::onReady(const dpp::ready_t &event, dpp::cluster &bot) -> void {
    m_timer.makeTask(1min, 1min, [this, &bot](){
        auto now = std::chrono::system_clock::now();
#if __cpp_lib_chrono >= 201907L
        auto tp = std::chrono::zoned_time{std::chrono::current_zone(), now}.get_local_time();
        auto day = std::chrono::floor<std::chrono::days>(tp);
        auto time = std::chrono::hh_mm_ss{std::chrono::floor<std::chrono::milliseconds>(tp-day)};
        auto ymd = std::chrono::year_month_day{day};

        auto m = std::uint32_t{ymd.month()};
        auto h = time.hours().count();
        auto min = time.minutes().count();
#else
        std::time_t tt = std::chrono::system_clock::to_time_t(now);
        std::tm utc_tm = *std::gmtime(&tt);
        std::tm local_tm = *std::localtime(&tt);

        auto m = local_tm.tm_mon + 1;
        auto h = local_tm.tm_hour;
        auto min = local_tm.tm_min;
#endif
        if(m == 10 && h == 0 && min == 0) {
            std::cout << m_current_word << std::endl;
                bot.message_create(dpp::message {
                        m_channel_id,
                        storm::core::format("A vos claviers ! Le thème du jour est \"{}\".", THEMES[m_current_word++])
                    },
                    [](const auto &event){
                        if(event.is_error()) elog("{}", event.http_info.body);
                    }
                );
            m_started = true;
        }
    });
}

/////////////////////////////////////
/////////////////////////////////////
auto GameOctoberPlugin::onMessageReceived(const dpp::message_create_t &event, dpp::cluster &bot) -> void {
    const auto &message = *event.msg;
    if(message.author->id == bot.me.id || message.channel_id != m_channel_id) return;

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
    if(m_started)
    bot.thread_create_with_message(
        storm::core::format("{}-{}-{}-gameoctober-2021", THEMES[m_current_word - 1u], name, d),
        m_channel_id,
        message.id,
        1440,
        [](const auto &event) {
            if(event.is_error()) elog("{}", event.http_info.body);
    });
}
