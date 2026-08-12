#include "Font_Manager.h"
#include "core/File_System.h"
#include "gui/Font_Manager_Config.h"
#include "gui/ImGuiEx.h"

#define ASSERT_FONT_MANAGER_IS_INITIALIZED() VERIFY(tools::g_font_manager != nullptr, "g_font_manager can't be found. Did you call font_manager_init() ?")

using namespace tools;

// private
namespace tools
{
    static Font_Manager* g_font_manager = nullptr;
}

Font_Manager* tools::font_manager_init(const Font_Manager_Config* config)
{
    VERIFY(g_font_manager == nullptr, "font_manager_init() called twice?");
    g_font_manager = new Font_Manager(config);
    
    for (const Font_Config& text_font : config->text)
    {
        font_manager_load(text_font);
    }

    // Assign text_fonts (user might want to change it later, but we need defaults)
    for( int each_slot = 0; each_slot < Font_Slot_COUNT; ++each_slot )
    {
        if(const char* font_name = config->defaults[each_slot] )
        {
            g_font_manager->fonts_by_slot[each_slot] = font_manager_get_by_name(font_name);
        }
        else
        {
            TOOLS_LOG(tools::Verbosity_Warning, "NodableView", "No default text_font declared for slot #%i, using ImGui's default text_font as fallback\n", each_slot);
            g_font_manager->fonts_by_slot[each_slot] = ImGui::GetDefaultFont();
        }
    }

    return g_font_manager;
}

Font_Manager* tools::font_manager()
{
    ASSERT_FONT_MANAGER_IS_INITIALIZED();
    return g_font_manager;
}

void tools::font_manager_shutdown()
{
    ASSERT_FONT_MANAGER_IS_INITIALIZED();
    delete g_font_manager;
    g_font_manager = nullptr;
}

ImFont* tools::font_manager_load(const Font_Config& font_config)
{
    ASSERT_FONT_MANAGER_IS_INITIALIZED();
    VERIFY(g_font_manager->fonts_by_name.find(font_config.id) == g_font_manager->fonts_by_name.end(), "use of same key for different fonts is not allowed");

    ImFont*   font     = nullptr;
    auto&     io       = ImGui::GetIO();

    // Create text_font
    {
        ImFontConfig imfont_cfg;
        imfont_cfg.RasterizerMultiply = 1.2f;
        imfont_cfg.OversampleH = 2;
        imfont_cfg.OversampleV = 3;
        Path absolute_path = Path::get_asset_path(font_config.path);
        TOOLS_LOG(tools::Verbosity_Diagnostic, "NodableView", "Adding text_font from file ... %s\n", absolute_path.c_str());
        font = io.Fonts->AddFontFromFileTTF(absolute_path.string().c_str(), font_config.size * g_font_manager->config->subsamples, &imfont_cfg);
    }

    // Add Icons my merging to previous text_font.
    if (font_config.icons_enable )
    {
        if(strlen( g_font_manager->config->icon.path) == 0)
        {
            TOOLS_LOG(tools::Verbosity_Warning, "NodableView", "config().font_manager.icon.path is empty, icons will be \"?\"\n");
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
        imfont_cfg.GlyphMinAdvanceX = font_config.icons_size * g_font_manager->config->subsamples; // monospace to fix text alignment in drop down menus.
        Path absolute_path = Path::get_asset_path(g_font_manager->config->icon.path);
        font = io.Fonts->AddFontFromFileTTF(absolute_path.string().c_str(), font_config.icons_size * g_font_manager->config->subsamples, &imfont_cfg, icons_ranges);
        TOOLS_LOG(tools::Verbosity_Diagnostic, "NodableView", "Merging icons font ...\n");
    }

    font->Scale = 1.0f / g_font_manager->config->subsamples;

    g_font_manager->fonts_by_name.insert_or_assign(font_config.id, font);
    TOOLS_LOG(tools::Verbosity_Diagnostic, "NodableView", "Font %s added: \"%s\"\n", font_config.id, font_config.path );
    return font;
}

ImFont* tools::font_manager_get_by_slot(Font_Slot slot)
{
    ASSERT_FONT_MANAGER_IS_INITIALIZED();
    return g_font_manager->fonts_by_slot.at(slot);
}

ImFont* tools::font_manager_get_by_name(const char *id)
{
    ASSERT_FONT_MANAGER_IS_INITIALIZED();
    return g_font_manager->fonts_by_name.at(id );
}

