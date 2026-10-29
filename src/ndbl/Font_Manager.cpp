#include "Font_Manager.h"
#include <imgui.h> // for ImFont
#include "Asserts.h"
#include "File_System.h"
#include "ImGuiEx.h"
#include "Log.h"

#define ASSERT_FONT_MANAGER_IS_INITIALIZED() VERIFY(g_font_manager != nullptr, "g_font_manager can't be found. Did you call font_manager_init() ?")

namespace ndbl
{

static Font_Manager* g_font_manager = nullptr;

Font_Manager* font_manager_init(const Font_Manager_Config* config)
{
    VERIFY(g_font_manager == nullptr, "font_manager_init() called twice?");

    g_font_manager = bdc::memory_new<Font_Manager>();
    g_font_manager->config = config;

    for(u32_t slot = 0; slot < Font_Slot_COUNT; ++slot )
    {
        if( slot == Font_Slot_Icons)
        {
            // Font_Slot_Icons don't need to be loaded, it is merged in other fonts on demand.
            continue;
        }

        const Font_Config& font_config = g_font_manager->config->fonts[slot];
        ImFont*            font        = font_manager_load(font_config); 
        assert(font != nullptr);
        g_font_manager->fonts_by_slot[slot] = font;
    }

    return g_font_manager;
}

Font_Manager* font_manager()
{
    ASSERT_FONT_MANAGER_IS_INITIALIZED();
    return g_font_manager;
}

void font_manager_shutdown()
{
    ASSERT_FONT_MANAGER_IS_INITIALIZED();
    bdc::memory_delete(g_font_manager);
    g_font_manager = nullptr;
}

ImFont* font_manager_load(const Font_Config& font_config)
{
    Font_Manager* font_manager = ndbl::font_manager();
    ImGuiIO&      io           = ImGui::GetIO();

    // Create text_font
    ImFontConfig imfont_cfg;
    imfont_cfg.RasterizerMultiply = 1.2f;
    imfont_cfg.OversampleH = 2;
    imfont_cfg.OversampleV = 3;
    Path absolute_path = Path::get_asset_path(font_config.path);
    NDBL_LOG(Verbosity_Diagnostic, "NodableView", "Adding text_font from file ... %s\n", absolute_path.c_str());
    ImFont* font = io.Fonts->AddFontFromFileTTF(absolute_path.c_str(), font_config.size * font_manager->config->subsamples, &imfont_cfg);
    assert(font != nullptr);

    // Add Icons my merging to previous text_font.
    if ( font_config.icons_enable )
    {
        const Font_Config& icon_font_config = font_manager->config->fonts[Font_Slot_Icons];
        assert(icon_font_config.id != font_config.id); // font config cannot merge itself!

        if( icon_font_config.path.size == 0)
        {
            NDBL_LOG(Verbosity_Warning, "NodableView", "config().font_manager.icon.path is empty, icons will be \"?\"\n");
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
        const float size = font_config.size * font_manager->config->icons_scale * font_manager->config->subsamples;
        imfont_cfg.GlyphMinAdvanceX = size; // monospace to fix text alignment in drop down menus.
        Path absolute_path = Path::get_asset_path(icon_font_config.path);
        font = io.Fonts->AddFontFromFileTTF(absolute_path.c_str(), size, &imfont_cfg, icons_ranges);
        assert(font != nullptr);
        NDBL_LOG(Verbosity_Diagnostic, "NodableView", "Merging icons font ...\n");
    }

    font->Scale = 1.0f / font_manager->config->subsamples;

    NDBL_LOG(Verbosity_Diagnostic, "NodableView", "Font %s added: \"%s\"\n", font_config.id.c_str(), font_config.path.c_str() );
    return font;
}

ImFont* font_manager_get_font(Font_Slot slot)
{
    return font_manager()->fonts_by_slot[slot];
}

} // namespace ndbl

