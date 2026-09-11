#pragma once

#include <SDL_keycode.h>
#include "bdc/String.hpp"

#include "ndbl/core/Event.h"

namespace ndbl
{
    typedef u16_t Condition_Flags;
    enum Condition_Flags_ : u16_t
    {
        Condition_Flags_DISABLE                             = 0,
        Condition_Flags_ENABLE_IF_HAS_SELECTION             = 1 << 0,
        Condition_Flags_ENABLE_IF_HAS_NO_SELECTION          = 1 << 1,
        Condition_Flags_ENABLE_IF_HAS_GRAPH                 = 1 << 3,
        Condition_Flags_DISABLE_IF_DRAGGING_THIS_SLOT       = 1 << 4,
        Condition_Flags_DISABLE_IF_DRAGGING_NON_THIS_SLOT   = 1 << 5,
        Condition_Flags_ONLY_FROM_GRAPH_EDITOR_CONTEXTUAL   = 1 << 6,
        Condition_Flags_ENABLE                              = Condition_Flags_ENABLE_IF_HAS_SELECTION
                                                            | Condition_Flags_ENABLE_IF_HAS_NO_SELECTION
                                                            | Condition_Flags_ENABLE_IF_HAS_GRAPH,
        Condition_Flags_HIGHLIGHTED_IN_GRAPH_EDITOR         = 1 << 10,
        Condition_Flags_HIGHLIGHTED_IN_TEXT_EDITOR          = 1 << 11,
        Condition_Flags_HIGHLIGHTED                         = Condition_Flags_HIGHLIGHTED_IN_GRAPH_EDITOR
                                                            | Condition_Flags_HIGHLIGHTED_IN_TEXT_EDITOR,
    };

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
        Event           event;
        bdc::String     label;
        Shortcut        shortcut;
        Condition_Flags condition_flags;
    };
}