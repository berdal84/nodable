#pragma once

#include <array>
#include <map>
#include <string>
#include <vector>

#include "FontManagerConfig.h"
#include "ImGuiEx.h"

namespace fw
{
    class FontManager
    {
    public:
        FontManager(const FontManagerConfig& config): m_config(config), m_fonts(), m_loaded_fonts() {}
        FontManager(const FontManager&) = delete;
        ~FontManager();;
        void        init();
        ImFont*     get_font(FontSlot) const;
        ImFont*     get_font(const char*) const;
    private:
        ImFont*     load_font(const FontConfig&);
        const FontManagerConfig&             m_config;
        std::array<ImFont*, FontSlot_COUNT>  m_fonts;        // Required fonts
        std::map<std::string, ImFont*>       m_loaded_fonts; // Available fonts
    };
}

