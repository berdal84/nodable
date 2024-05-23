#pragma once

#include <array>
#include <map>
#include <string>
#include <vector>

#include "FontManagerConfig.h"
#include "ImGuiEx.h"

namespace tools
{
    class FontManager
    {
    public:
        FontManager(): m_fonts(), m_loaded_fonts() {}
        FontManager(const FontManager&) = delete;
        ~FontManager();;
        void        init();
        ImFont*     get_font(FontSlot) const;
        ImFont*     get_font(const char*) const;
    private:
        ImFont*     load_font(const FontConfig&);
        std::array<ImFont*, FontSlot_COUNT>  m_fonts;        // Required fonts
        std::map<std::string, ImFont*>       m_loaded_fonts; // Available fonts
    };
}

