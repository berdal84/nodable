#pragma once

#include <SDL_keycode.h>
#include "bdc/String.hpp"

#include "tools/core/Event.h"

namespace tools
{
    /** Data describing a shortcut (ex: "Reset": Ctrl + Alt + R) */
    struct Shortcut
    {
        SDL_Keycode key         = SDLK_UNKNOWN;     // a key to be pressed
        SDL_Keymod  mod         = KMOD_NONE;        // modifiers (alt, ctrl, etc.)
        bdc::String description;
        bdc::String to_string() const;              // TODO: this could be a precomputed
    };

    struct Action 
    {
        Event       event;
        bdc::String label;
        Shortcut    shortcut;
        u64_t       flags;
    };
}