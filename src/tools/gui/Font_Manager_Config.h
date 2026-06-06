#pragma once

#include <vector>
#include <array>
#include "core/reflection/Enum.h"

namespace tools
{
    struct Font_Config // Struct to store text_font configuration
    {
        const char*  id;           // Font identifier
        const char*  path;         // Font path relative to application folder
        float        size;         // Font size in px
        bool         icons_enable; // If true, icons will be merged to the text_font
        float        icons_size;   // If icons_enable is true, this will define icons size
    };

    enum Font_Slot
    {
        Font_Slot_Paragraph = 0,
        Font_Slot_Heading,
        Font_Slot_Code,
        Font_Slot_ToolBtn,
        Font_Slot_COUNT
    };

    REFLECT_ENUM(Font_Slot)
    (
        REFLECT_ENUM_V(Font_Slot_Paragraph)
        REFLECT_ENUM_V(Font_Slot_Heading)
        REFLECT_ENUM_V(Font_Slot_Code)
        REFLECT_ENUM_V(Font_Slot_ToolBtn)
    )

    struct Font_Manager_Config
    {
        std::vector<Font_Config>                 text;      // text fonts
        std::array<const char*, Font_Slot_COUNT> defaults;  // ids for font slots
        Font_Config                              icon;      // icon font
        float                                    subsamples;
    };
}