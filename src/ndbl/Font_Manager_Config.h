#pragma once

#include "bdc/String.hpp"
#include "bdc/Array.hpp"
#include "reflection/Enum.h"

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
}