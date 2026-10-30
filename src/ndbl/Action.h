#pragma once

#include <SDL_keycode.h>

#include "bdc/String.hpp"
#include "Event.h"

namespace ndbl
{
    typedef u16_t Action_Flags;
    enum Action_Flags_ : u16_t
    {
        Action_Flags_DISABLE                             = 0,
        Action_Flags_ENABLE_IF_HAS_SELECTION             = 1 << 0,
        Action_Flags_ENABLE_IF_HAS_NO_SELECTION          = 1 << 1,
        Action_Flags_ENABLE_IF_HAS_GRAPH                 = 1 << 3,
        Action_Flags_DISABLE_IF_DRAGGING_THIS_SLOT       = 1 << 4,
        Action_Flags_DISABLE_IF_DRAGGING_NON_THIS_SLOT   = 1 << 5,
        Action_Flags_ONLY_FROM_GRAPH_EDITOR_CONTEXTUAL   = 1 << 6,
        Action_Flags_ENABLE                              = Action_Flags_ENABLE_IF_HAS_SELECTION
                                                         | Action_Flags_ENABLE_IF_HAS_NO_SELECTION
                                                         | Action_Flags_ENABLE_IF_HAS_GRAPH,
        Action_Flags_HIGHLIGHTED_IN_GRAPH_EDITOR         = 1 << 10,
        Action_Flags_HIGHLIGHTED_IN_TEXT_EDITOR          = 1 << 11,
        Action_Flags_HIGHLIGHTED                         = Action_Flags_HIGHLIGHTED_IN_GRAPH_EDITOR
                                                         | Action_Flags_HIGHLIGHTED_IN_TEXT_EDITOR,
        Action_Flags_DISABLE_INDEXING                    = 1 << 12,
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
        Action_Flags    flags;
    };
}