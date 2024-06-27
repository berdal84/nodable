#pragma once

#include <array>
#include <map>
#include <string>
#include <vector>
#include "FontManagerConfig.h"
#include "imgui.h" // for ImFont

namespace tools
{
    class FontManager
    {
    public:
        ImFont*     get_font(FontSlot) const;
        ImFont*     get_font(const char*) const;
        void        init(const FontManagerConfig* config);
    private:
        ImFont*     load_font(const FontConfig&);
        const FontManagerConfig*             m_config = nullptr; // will be assigned by init()
        std::array<ImFont*, FontSlot_COUNT>  m_fonts  = {nullptr, nullptr, nullptr, nullptr}; // Font required, user can get_font by name or by slot
        std::map<std::string, ImFont*>       m_loaded_fonts; // All the fonts loaded in memory
    };

    // singleton-like global functions

    [[nodiscard]]
    FontManager* init_font_manager(); // note: store the pointer to shut it down later
    FontManager* get_font_manager(); // require to call init first
    void         shutdown_font_manager(FontManager*); // undo init
}

