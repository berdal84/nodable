#pragma once

#include <SDL_keycode.h>
#include <nodable/app/Event.h>

namespace ndbl
{
    struct Shortcut
    {
        SDL_Keycode key    = SDLK_UNKNOWN;    // a key to be pressed
        SDL_Keymod  mod    = KMOD_NONE;       // modifiers (alt, ctrl, etc.)
        EventType   event  = EventType::none; // event to trigger
    };
}