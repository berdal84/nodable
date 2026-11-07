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

        Action_Flags_DISABLE_IF_DRAGGING_THIS_SLOT       = 1 << 1,
        Action_Flags_DISABLE_IF_DRAGGING_NON_THIS_SLOT   = 1 << 2,

        Action_Flags_ENABLE_IF_HAS_SELECTION             = 1 << 3,
        Action_Flags_ENABLE_IF_HAS_NO_SELECTION          = 1 << 4,
        Action_Flags_ENABLE_IF_HAS_GRAPH                 = 1 << 5,
        Action_Flags_ALWAYS_ENABLE                       = Action_Flags_ENABLE_IF_HAS_SELECTION
                                                         | Action_Flags_ENABLE_IF_HAS_NO_SELECTION
                                                         | Action_Flags_ENABLE_IF_HAS_GRAPH,

        Action_Flags_SHOW_IN_GRAPH_EDITOR_CTX_MENU_ONLY  = 1 << 6,
        Action_Flags_SHOW_ON_GRAPH_EDITOR_GUI            = 1 << 7,
        Action_Flags_SHOW_ON_TEXT_EDITOR_GUI             = 1 << 8,
        Action_Flags_SHOW_ON_GUI                         = Action_Flags_SHOW_ON_GRAPH_EDITOR_GUI
                                                         | Action_Flags_SHOW_ON_TEXT_EDITOR_GUI,
        Action_Flags_DISABLE_INDEXING                    = 1 << 9, // when ON, action cannot be triggered automatically when pressing keyboard
    };

    /** Data describing a shortcut (ex: "Reset": Ctrl + Alt + R) */
    struct Shortcut
    {
        SDL_Keycode key         = SDLK_UNKNOWN;     // a key to be pressed
        SDL_Keymod  mod         = KMOD_NONE;        // modifiers (alt, ctrl, etc.)
        bdc::String description;
    };

    bdc::String shortcut_to_tstring(const Shortcut&); // Return a string including shortcut and description (ex: "Ctrl + Alt + M Do this")

    struct Action 
    {
        Event           event;
        bdc::String     label;
        Shortcut        shortcut;
        Action_Flags    flags;
    };
}