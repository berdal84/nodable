#pragma once

#include "bdc/String.hpp"
#include "Font_Manager_Config.h"

struct ImFont;

namespace ndbl
{
    struct Font_Manager
    {
        const Font_Manager_Config*  config;
        ImFont*                     fonts_by_slot[Font_Slot_COUNT] = {nullptr}; // Font required, user can get_font by name or by slot
    };

    // singleton-like global functions

    Font_Manager*   font_manager_init(const Font_Manager_Config* config);
    void            font_manager_shutdown(); // undo init
    Font_Manager*   font_manager();          // require to call init first
    ImFont*         font_manager_get_font(Font_Slot);
    ImFont*         font_manager_load(const Font_Config&);
}

