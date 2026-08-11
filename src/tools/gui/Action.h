#pragma once

#include <SDL_keycode.h>
#include <string>

#include "tools/core/Event.h"

namespace tools
{
    /** Data describing a shortcut (ex: "Reset": Ctrl + Alt + R) */
    struct Shortcut
    {
        SDL_Keycode key         = SDLK_UNKNOWN;     // a key to be pressed
        SDL_Keymod  mod         = KMOD_NONE;        // modifiers (alt, ctrl, etc.)
        std::string description;                    // TODO: this could be a const char*
        std::string to_string() const;              // TODO: this could be a precomputed
    };

    struct Action 
    {
        Action(
            Event       _event,
            const char* _label      = "action",
            Shortcut    _shortcut   = {},
            u64_t       _flags      = 0
        )
        : label(_label)
        , event(_event)
        , shortcut(_shortcut)
        , flags(_flags)
        {}

        Event       event;
        std::string label; // TODO: this could be a const char*
        Shortcut    shortcut;
        u64_t       flags;

    };
}