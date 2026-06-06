#pragma once

#include <array>
#include <map>
#include <string>
#include <vector>
#include "Font_Manager_Config.h"
#include "imgui.h" // for ImFont

namespace tools
{
    class Font_Manager
    {
    public:
        ImFont*     get_font(Font_Slot) const;
        ImFont*     get_font(const char*) const;
        void        init(const Font_Manager_Config* config);
    private:
        ImFont*     load_font(const Font_Config&);
        const Font_Manager_Config*             m_config = nullptr; // will be assigned by init()
        std::array<ImFont*, Font_Slot_COUNT>  m_fonts  = {nullptr, nullptr, nullptr, nullptr}; // Font required, user can get_font by name or by slot
        std::map<std::string, ImFont*>       m_loaded_fonts; // All the fonts loaded in memory
    };

    // singleton-like global functions

    [[nodiscard]]
    Font_Manager* init_font_manager(); // note: store the pointer to shut it down later
    Font_Manager* get_font_manager(); // require to call init first
    void         shutdown_font_manager(Font_Manager*); // undo init
}

