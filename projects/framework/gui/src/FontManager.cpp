#include <fw/gui/FontManager.h>
#include <fw/core/log.h>
#include <imgui/imgui_internal.h>
#include <fw/core/assertions.h>
#include <fw/gui/App.h>

using namespace fw;

void FontManager::init(
        const std::vector<FontConf>& text_fonts,
        const std::array<const char*, FontSlot_COUNT>& defaults,
        const FontConf* icon_font)
{
    for ( const FontConf& each_font : text_fonts)
    {
        load_font(&each_font, icon_font);
    }

    // Assign text_fonts (user might want to change it later, but we need defaults)
    for( int each_slot = 0; each_slot < fw::FontSlot_COUNT; ++each_slot )
    {
        if(auto font = defaults[each_slot] )
        {
            m_fonts[each_slot] = get_font_by_id( font );
        }
        else
        {
            LOG_WARNING("AppView", "No default text_font declared for slot #%i, using ImGui's default text_font as fallback\n", each_slot);
            m_fonts[each_slot] = ImGui::GetDefaultFont();
        }
    }
}

ImFont* FontManager::get_font_by_id(const char *id)const
{
    return m_loaded_fonts.at(id );
}

ImFont* FontManager::load_font(const FontConf* text_font, const FontConf* icon_font)
{
    FW_ASSERT(m_loaded_fonts.find(text_font->id) == m_loaded_fonts.end()); // do not allow the use of same key for different fonts

    ImFont*   font     = nullptr;
    auto&     io       = ImGui::GetIO();

    // Create text_font
    {
        ImFontConfig config;
        config.OversampleH = 3;
        config.OversampleV = 1;

        //io.Fonts->AddFontDefault();
        std::string fontPath = App::to_absolute_asset_path(text_font->path);
        LOG_VERBOSE("AppView", "Adding text_font from file ... %s\n", fontPath.c_str())
        font = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), text_font->size, &config);
    }

    // Add Icons my merging to previous text_font.
    if (text_font->icons_enable )
    {
        if(strlen(icon_font->path) == 0)
        {
            LOG_WARNING("AppView", "m_conf.icons is empty, icons will be \"?\"\n");
            return font;
        }

        // merge in icons text_font
        static const ImWchar icons_ranges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
        ImFontConfig config;
        config.OversampleH = 3;
        config.OversampleV = 1;
        config.MergeMode   = true;
        config.PixelSnapH  = true;
        config.GlyphOffset.y = -(text_font->icons_size - text_font->size)*0.5f;
        config.GlyphMinAdvanceX = text_font->icons_size; // monospace to fix text alignment in drop down menus.
        auto fontPath = App::to_absolute_asset_path(icon_font->path);
        font = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), text_font->icons_size, &config, icons_ranges);
        LOG_VERBOSE("AppView", "Adding icons to text_font ...\n")
    }

    m_loaded_fonts.insert_or_assign(text_font->id, font);
    LOG_MESSAGE("AppView", "Font %s added to register with the id \"%s\"\n", text_font->path, text_font->id)
    return font;
}

ImFont* FontManager::get_font(FontSlot slot) const
{
    return m_fonts[slot];
}

