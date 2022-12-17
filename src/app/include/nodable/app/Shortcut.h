#pragma once

#include <SDL_keycode.h>
#include <nodable/app/Event.h>
#include <array>
#include <string>

namespace ndbl
{
    struct Shortcut
    {
        SDL_Keycode key         = SDLK_UNKNOWN;    // a key to be pressed
        SDL_Keymod  mod         = KMOD_NONE;       // modifiers (alt, ctrl, etc.)
        std::string description;
        std::string to_string() const;
    };
}

