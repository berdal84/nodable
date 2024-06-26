#include "FontManager.h"
#include "App.h"
#include "Config.h"
#include "gui.h"

using namespace tools;

FontManager::~FontManager()
{ LOG_VERBOSE("tools::FontManager", "Destructor " OK "\n"); }

void FontManager::init()
{
    for (const FontConfig& each_font : g_conf->font_manager.text)
    {
        load_font(each_font);
    }

    // Assign text_fonts (user might want to change it later, but we need defaults)
    for( int each_slot = 0; each_slot < FontSlot_COUNT; ++each_slot )
    {
        if(auto font = g_conf->font_manager.defaults[each_slot] )
        {
            m_fonts[each_slot] = get_font(font);
        }
        else
        {
            LOG_WARNING("NodableView", "No default text_font declared for slot #%i, using ImGui's default text_font as fallback\n", each_slot);
            m_fonts[each_slot] = ImGui::GetDefaultFont();
        }
    }
}

ImFont* FontManager::load_font(const FontConfig& text_font)
{
    EXPECT(m_loaded_fonts.find(text_font.id) == m_loaded_fonts.end(), "use of same key for different fonts is not allowed");

    ImFont*   font     = nullptr;
    auto&     io       = ImGui::GetIO();

    // Create text_font
    {
        ImFontConfig imfont_cfg;
        imfont_cfg.RasterizerMultiply = 1.2f;
        imfont_cfg.OversampleH = 2;
        imfont_cfg.OversampleV = 3;
        std::filesystem::path absolute_path = App::asset_path(text_font.path);
        LOG_VERBOSE("NodableView", "Adding text_font from file ... %s\n", absolute_path.c_str())
        font = io.Fonts->AddFontFromFileTTF(absolute_path.string().c_str(), text_font.size * g_conf->font_manager.subsamples, &imfont_cfg);
    }

    // Add Icons my merging to previous text_font.
    if (text_font.icons_enable )
    {
        if(strlen( g_conf->font_manager.icon.path) == 0)
        {
            LOG_WARNING("NodableView", "g_conf->font_manager.icon.path is empty, icons will be \"?\"\n");
            return font;
        }

        // merge in icons text_font
        static const ImWchar icons_ranges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
        ImFontConfig imfont_cfg;

        imfont_cfg.MergeMode   = true;
        imfont_cfg.RasterizerMultiply = 1.2f;
        imfont_cfg.OversampleH = 2;
        imfont_cfg.OversampleV = 3;
        //imfont_cfg.GlyphOffset.y = -(text_font.icons_size - text_font.size)/2.f;
        imfont_cfg.GlyphMinAdvanceX = text_font.icons_size  * g_conf->font_manager.subsamples; // monospace to fix text alignment in drop down menus.
        std::filesystem::path absolute_path = App::asset_path( g_conf->font_manager.icon.path);
        font = io.Fonts->AddFontFromFileTTF(absolute_path.string().c_str(), text_font.icons_size * g_conf->font_manager.subsamples, &imfont_cfg, icons_ranges);
        LOG_VERBOSE("NodableView", "Merging icons font ...\n")
    }

    font->Scale = 1.0f / g_conf->font_manager.subsamples;

    m_loaded_fonts.insert_or_assign(text_font.id, font);
    LOG_MESSAGE("NodableView", "Font %s added: \"%s\"\n", text_font.id, text_font.path )
    return font;
}

ImFont* FontManager::get_font(FontSlot slot) const
{
    return m_fonts[slot];
}

ImFont* FontManager::get_font(const char *id)const
{
    return m_loaded_fonts.at(id );
}

