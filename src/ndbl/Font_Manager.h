#pragma once
#include "bdc/String.hpp"
#include "bdc/Array.hpp"
#include "reflection/Enum.h"

struct ImFont;

namespace ndbl
{
    struct Font_Config // Struct to store text_font configuration
    {
        bdc::String  id;           // Font identifier
        bdc::String  path;         // Font path relative to application folder
        float        size;         // Font size in px
        bool         icons_enable; // If true, icons will be merged to the text_font
    };

    enum Font_Slot
    {
        Font_Slot_Default = 0,
        Font_Slot_Heading,
        Font_Slot_ToolBtn,
        Font_Slot_Icons,
        
        Font_Slot_COUNT
    };

    REFLECT_ENUM(Font_Slot)
    (
        REFLECT_ENUM_V(Font_Slot_Default)
        REFLECT_ENUM_V(Font_Slot_Heading)
        REFLECT_ENUM_V(Font_Slot_ToolBtn)
        REFLECT_ENUM_V(Font_Slot_Icons)
    )

    struct Font_Manager_Config
    {
        Font_Config fonts[Font_Slot_COUNT];
        float       subsamples;
        float       icons_scale;
    };

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

