#pragma once

#include <array>
#include <map>
#include <string>
#include "Font_Manager_Config.h"
#include "imgui.h" // for ImFont

namespace tools
{
    struct Font_Manager
    {
        const Font_Manager_Config*              config = nullptr; // will be assigned by init()
        std::array<ImFont*, Font_Slot_COUNT>    fonts_by_slot  = {nullptr, nullptr, nullptr, nullptr}; // Font required, user can get_font by name or by slot
        std::map<std::string, ImFont*>          fonts_by_name; // All the fonts loaded in memory

        Font_Manager() = delete;
        
        Font_Manager(const Font_Manager_Config* _config)
        : config(_config)
        {}
    };

    // singleton-like global functions

    Font_Manager*   font_manager_init(const Font_Manager_Config* config);
    void            font_manager_shutdown(); // undo init
    Font_Manager*   font_manager();          // require to call init first
    ImFont*         font_manager_get_by_slot(Font_Slot);
    ImFont*         font_manager_get_by_name(const char*);
    ImFont*         font_manager_load(const Font_Config&);
}

