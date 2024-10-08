#pragma once
#include "tools/core/reflection/reflection"

namespace tools
{
    struct FontConfig // Struct to store text_font configuration
    {
        const char*  id;           // Font identifier
        const char*  path;         // Font path relative to application folder
        float        size;         // Font size in px
        bool         icons_enable; // If true, icons will be merged to the text_font
        float        icons_size;   // If icons_enable is true, this will define icons size
    };

    enum FontSlot
    {
        FontSlot_Paragraph = 0,
        FontSlot_Heading,
        FontSlot_Code,
        FontSlot_ToolBtn,
        FontSlot_COUNT
    };

    REFLECT_ENUM(FontSlot)
    REFLECT_ENUM_VALUE(FontSlot_Paragraph)
    REFLECT_ENUM_VALUE(FontSlot_Heading)
    REFLECT_ENUM_VALUE(FontSlot_Code)
    REFLECT_ENUM_VALUE(FontSlot_ToolBtn)
    REFLECT_ENUM_END

    struct FontManagerConfig
    {
        std::vector<FontConfig>                 text;      // text fonts
        std::array<const char*, FontSlot_COUNT> defaults;  // ids for font slots
        FontConfig                              icon;      // icon font
        float                                   subsamples;
    };
}