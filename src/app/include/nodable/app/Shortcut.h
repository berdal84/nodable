#pragma once

#include <SDL_keycode.h>
#include <nodable/app/Event.h>
#include <array>
#include <string>

namespace ndbl
{
    enum Condition {
        Condition_NEVER = 0,
        Condition_ALWAYS = 1 << 0,
        Condition_HAS_SELELECTION = 1 << 1,
        Condition_HAS_NO_SELECTION = 1 << 2
    };

    struct Shortcut
    {
        SDL_Keycode key    = SDLK_UNKNOWN;    // a key to be pressed
        SDL_Keymod  mod    = KMOD_NONE;       // modifiers (alt, ctrl, etc.)
        EventType   event  = EventType::none; // event to trigger
        Condition   cond   = Condition_NEVER;
        std::string label;
        std::string to_string(size_t label_max_length = 16) const;
    };

    class ShortcutManager {
    public:
         static std::vector<Shortcut> s_shortcuts;
    };
}

