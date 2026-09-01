#pragma once

#include <string>
#include <unordered_map>

// Maps common GitHub-style :shortcode: names to their UTF-8 emoji glyph.
// Not exhaustive -- covers the emoji people actually type in markdown.
// Literal emoji already present in the source file need no translation at
// all, since they pass through untouched in a UTF-8 terminal.
namespace emoji {

inline const std::unordered_map<std::string, std::string>& shortcode_map() {
    static const std::unordered_map<std::string, std::string> table = {
        // faces
        {"smile", "\xF0\x9F\x98\x84"},
        {"smiley", "\xF0\x9F\x98\x83"},
        {"laughing", "\xF0\x9F\x98\x86"},
        {"blush", "\xF0\x9F\x98\x8A"},
        {"relaxed", "\xE2\x98\xBA\xEF\xB8\x8F"},
        {"wink", "\xF0\x9F\x98\x89"},
        {"heart_eyes", "\xF0\x9F\x98\x8D"},
        {"kissing_heart", "\xF0\x9F\x98\x98"},
        {"thinking", "\xF0\x9F\xA4\x94"},
        {"neutral_face", "\xF0\x9F\x98\x90"},
        {"expressionless", "\xF0\x9F\x98\x91"},
        {"no_mouth", "\xF0\x9F\x98\xB6"},
        {"rolling_eyes", "\xF0\x9F\x99\x84"},
        {"cry", "\xF0\x9F\x98\xA2"},
        {"sob", "\xF0\x9F\x98\xAD"},
        {"joy", "\xF0\x9F\x98\x82"},
        {"scream", "\xF0\x9F\x98\xB1"},
        {"angry", "\xF0\x9F\x98\xA0"},
        {"rage", "\xF0\x9F\x98\xA1"},
        {"confused", "\xF0\x9F\x98\x95"},
        {"sweat", "\xF0\x9F\x98\x93"},
        {"sleepy", "\xF0\x9F\x98\xAA"},
        {"sleeping", "\xF0\x9F\x98\xB4"},
        {"grin", "\xF0\x9F\x98\x81"},
        {"grinning", "\xF0\x9F\x98\x80"},
        {"innocent", "\xF0\x9F\x98\x87"},
        {"sunglasses", "\xF0\x9F\x98\x8E"},

        // hands / gestures
        {"thumbsup", "\xF0\x9F\x91\x8D"},
        {"+1", "\xF0\x9F\x91\x8D"},
        {"thumbsdown", "\xF0\x9F\x91\x8E"},
        {"-1", "\xF0\x9F\x91\x8E"},
        {"clap", "\xF0\x9F\x91\x8F"},
        {"wave", "\xF0\x9F\x91\x8B"},
        {"pray", "\xF0\x9F\x99\x8F"},
        {"muscle", "\xF0\x9F\x92\xAA"},
        {"ok_hand", "\xF0\x9F\x91\x8C"},
        {"point_right", "\xF0\x9F\x91\x89"},
        {"point_left", "\xF0\x9F\x91\x88"},
        {"point_up", "\xF0\x9F\x91\x86"},
        {"point_down", "\xF0\x9F\x91\x87"},
        {"raised_hands", "\xF0\x9F\x99\x8C"},
        {"handshake", "\xF0\x9F\xA4\x9D"},

        // symbols / effects
        {"fire", "\xF0\x9F\x94\xA5"},
        {"star", "\xE2\xAD\x90"},
        {"star2", "\xF0\x9F\x8C\x9F"},
        {"sparkles", "\xE2\x9C\xA8"},
        {"zap", "\xE2\x9A\xA1"},
        {"boom", "\xF0\x9F\x92\xA5"},
        {"tada", "\xF0\x9F\x8E\x89"},
        {"confetti_ball", "\xF0\x9F\x8E\x8A"},
        {"balloon", "\xF0\x9F\x8E\x88"},
        {"gift", "\xF0\x9F\x8E\x81"},
        {"heart", "\xE2\x9D\xA4\xEF\xB8\x8F"},
        {"broken_heart", "\xF0\x9F\x92\x94"},
        {"two_hearts", "\xF0\x9F\x92\x95"},
        {"heartbeat", "\xF0\x9F\x92\x93"},
        {"heartpulse", "\xF0\x9F\x92\x97"},
        {"warning", "\xE2\x9A\xA0\xEF\xB8\x8F"},
        {"exclamation", "\xE2\x9D\x97"},
        {"question", "\xE2\x9D\x93"},
        {"white_check_mark", "\xE2\x9C\x85"},
        {"heavy_check_mark", "\xE2\x9C\x94\xEF\xB8\x8F"},
        {"x", "\xE2\x9D\x8C"},
        {"o", "\xE2\xAD\x95"},
        {"bulb", "\xF0\x9F\x92\xA1"},
        {"bomb", "\xF0\x9F\x92\xA3"},
        {"gear", "\xE2\x9A\x99\xEF\xB8\x8F"},
        {"lock", "\xF0\x9F\x94\x92"},
        {"unlock", "\xF0\x9F\x94\x93"},
        {"key", "\xF0\x9F\x94\x91"},
        {"hammer", "\xF0\x9F\x94\xA8"},
        {"wrench", "\xF0\x9F\x94\xA7"},
        {"mag", "\xF0\x9F\x94\x8D"},
        {"pushpin", "\xF0\x9F\x93\x8C"},
        {"bookmark", "\xF0\x9F\x94\x96"},

        // objects / docs
        {"book", "\xF0\x9F\x93\x96"},
        {"books", "\xF0\x9F\x93\x9A"},
        {"pencil", "\xF0\x9F\x93\x9D"},
        {"memo", "\xF0\x9F\x93\x9D"},
        {"email", "\xF0\x9F\x93\xA7"},
        {"envelope", "\xE2\x9C\x89\xEF\xB8\x8F"},
        {"inbox_tray", "\xF0\x9F\x93\xA5"},
        {"outbox_tray", "\xF0\x9F\x93\xA4"},
        {"calendar", "\xF0\x9F\x93\x85"},
        {"clock", "\xF0\x9F\x95\x90"},
        {"hourglass", "\xE2\x8F\xB3"},
        {"package", "\xF0\x9F\x93\xA6"},
        {"computer", "\xF0\x9F\x92\xBB"},
        {"desktop_computer", "\xF0\x9F\x96\xA5\xEF\xB8\x8F"},
        {"keyboard", "\xE2\x8C\xA8\xEF\xB8\x8F"},
        {"phone", "\xF0\x9F\x93\xB1"},
        {"battery", "\xF0\x9F\x94\x8B"},
        {"bug", "\xF0\x9F\x90\x9B"},

        // travel
        {"rocket", "\xF0\x9F\x9A\x80"},
        {"airplane", "\xE2\x9C\x88\xEF\xB8\x8F"},
        {"car", "\xF0\x9F\x9A\x97"},
        {"bus", "\xF0\x9F\x9A\x8C"},
        {"train", "\xF0\x9F\x9A\x86"},
        {"ship", "\xF0\x9F\x9A\xA2"},
        {"bike", "\xF0\x9F\x9A\xB2"},

        // weather / nature
        {"sunny", "\xE2\x98\x80\xEF\xB8\x8F"},
        {"cloud", "\xE2\x98\x81\xEF\xB8\x8F"},
        {"rain", "\xF0\x9F\x8C\xA7"},
        {"snowflake", "\xE2\x9D\x84\xEF\xB8\x8F"},
        {"umbrella", "\xE2\x98\x82\xEF\xB8\x8F"},
        {"rainbow", "\xF0\x9F\x8C\x88"},
        {"earth_americas", "\xF0\x9F\x8C\x8E"},
        {"moon", "\xF0\x9F\x8C\x99"},

        // food
        {"coffee", "\xE2\x98\x95"},
        {"beer", "\xF0\x9F\x8D\xBA"},
        {"pizza", "\xF0\x9F\x8D\x95"},
        {"hamburger", "\xF0\x9F\x8D\x94"},
        {"apple", "\xF0\x9F\x8D\x8E"},
        {"cake", "\xF0\x9F\x8E\x82"},

        // animals
        {"dog", "\xF0\x9F\x90\xB6"},
        {"cat", "\xF0\x9F\x90\xB1"},
        {"mouse", "\xF0\x9F\x90\xAD"},
        {"panda_face", "\xF0\x9F\x90\xBC"},
        {"bird", "\xF0\x9F\x90\xA6"},
        {"fish", "\xF0\x9F\x90\x9F"},
        {"snake", "\xF0\x9F\x90\x8D"},
        {"turtle", "\xF0\x9F\x90\xA2"},

        // misc / achievement
        {"100", "\xF0\x9F\x92\xAF"},
        {"trophy", "\xF0\x9F\x8F\x86"},
        {"medal", "\xF0\x9F\x8F\x85"},
        {"checkered_flag", "\xF0\x9F\x8F\x81"},
        {"crown", "\xF0\x9F\x91\x91"},
        {"eyes", "\xF0\x9F\x91\x80"},
        {"speech_balloon", "\xF0\x9F\x92\xAC"},
        {"thought_balloon", "\xF0\x9F\x92\xAD"},
    };
    return table;
}

}  // namespace emoji
