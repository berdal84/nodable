#pragma once
#include <vector>
#include <array>
#include <map>
#include <string>
#include <imgui/imgui.h>
#include <fw/core/reflection/enum.h>
#include <fw/core/log.h>

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
        struct Config {
            std::vector<FontConf>                   text;      // text fonts
            std::array<const char*, FontSlot_COUNT> defaults;  // ids for font slots
            FontConf                                icon;      // icon font
            float                                   subsamples;
        };

        FontManager(Config& config): m_config(config), m_fonts(), m_loaded_fonts() {}
        FontManager(const FontManager&) = delete;
        ~FontManager() { LOG_VERBOSE("fw::FontManager", "Destructor " OK "\n"); };
        void        init();
        ImFont*     get_font(FontSlot) const;
        ImFont*     get_font(const char*) const;
    private:
        ImFont*     load_font(const FontConf&);
        Config&                              m_config;
        std::array<ImFont*, FontSlot_COUNT>  m_fonts;        // Required fonts
        std::map<std::string, ImFont*>       m_loaded_fonts; // Available fonts
    };
}

