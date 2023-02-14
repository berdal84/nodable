#pragma once
#include <vector>
#include <array>
#include <map>
#include <string>
#include <imgui/imgui.h>
#include <fw/core/reflection/enum.h>

namespace fw {

    enum FontSlot {
        FontSlot_Paragraph,
        FontSlot_Heading,
        FontSlot_Code,
        FontSlot_ToolBtn,
        FontSlot_COUNT
    };

    R_ENUM(FontSlot)
        R_ENUM_VALUE(FontSlot_Paragraph)
        R_ENUM_VALUE(FontSlot_Heading)
        R_ENUM_VALUE(FontSlot_Code)
        R_ENUM_VALUE(FontSlot_ToolBtn)
    R_ENUM_END

    struct FontConf              // Struct to store text_font configuration
    {
        const char*  id;           // Font identifier
        const char*  path;         // Font path relative to application folder
        float        size;         // Font size in px
        bool         icons_enable; // If true, icons will be merged to the text_font
        float        icons_size;   // If icons_enable is true, this will define icons size
    };

    class FontManager
    {
    public:
        void        init( const std::vector<FontConf>&text_fonts,
                          const std::array<const char*, FontSlot_COUNT>& defaults,
                          const FontConf* icon_font);
        ImFont*           get_font(FontSlot) const;
        ImFont*           get_font_by_id(const char*) const;
        ImFont*           load_font(const FontConf* font, const FontConf* icon_font);
    private:
        std::array<ImFont*, FontSlot_COUNT>  m_fonts;        // Required fonts
        std::map<std::string, ImFont*>       m_loaded_fonts; // Available fonts
    };
}

