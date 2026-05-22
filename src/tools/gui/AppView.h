#pragma once

#include <SDL.h>
#include <vector>
#include <array>
#include "gui/ActionManager.h"
#include "imgui.h"
#include "tools/core/FileSystem.h"
#include <string>
#include "tools/core/types.h"
#include "tools/core/Signals.h"

namespace tools
{
    // forward declarations
    class AppState;    
    class TextureManager;
    class EventManager;
    class FontManager;
    class VirtualMachine;

    enum DialogType // Helps to configure the file browse dialog
    {
        DIALOG_SaveAs,   // Allows to set a new file or select an existing file
        DIALOG_Browse    // Only allows to pick a file
    };

    /*
        * Enum to identify dockspaces
        *
        -------------------------------------
        |                TOP                |
        |-----------------------------------|
        |           CENTER         |  RIGHT |
        |-----------------------------------|
        |              BOTTOM               |
        ------------------------------------
        */
    enum Dockspace
    {
        Dockspace_ROOT,
        Dockspace_CENTER,
        Dockspace_RIGHT,
        Dockspace_BOTTOM,
        Dockspace_TOP,
        Dockspace_COUNT,
    };

	struct AppViewState
	{
        tools::SimpleSignal signal_reset_layout; // add custom code during layout reset
        tools::SimpleSignal signal_draw_splashscreen_content; // to insert custom code into the splashscreen window

        std::string         title;
        TextureManager*     texture_manager  = nullptr;
        FontManager*        font_manager     = nullptr;
        EventManager*       event_manager    = nullptr;
        ActionManager*      action_manager   = nullptr;
        SDL_GLContext       sdl_gl_context   = nullptr;
        SDL_Window*         sdl_window       = nullptr;
        u32_t               ticks        = 0;
        u32_t               dt_in_ms         = 1000 / 30;
        float               dt_in_s          = 1.f/30.f;
        float               smoothed_fps     = 30.f;
        AppState*           app              = nullptr;
        bool                is_layout_initialized = false;
        bool                show_splashscreen = true; // flag to show/hide splashscreen
        std::array<ImGuiID, Dockspace_COUNT>
                            dockspaces{};
    };

    void        appview_init(AppViewState*, AppState*);
    void        appview_update(AppViewState*);
    void        appview_begin(AppViewState*);
    void        appview_end(AppViewState*);
    void        appview_shutdown(AppViewState*);
    void        appview_draw_splashscreen(AppViewState*); // If needed, use begin/end_splashscreen static methods to override this. Ex: if ( AppView::begin_splashscreen(m_app->config) ) { /* your code here */; AppView::end_splashscreen(); }
    ImGuiID     appview_get_dockspace(AppViewState*, Dockspace);
    int         appview_fps(AppViewState*);      // get the current frame per second (un-smoothed)
    void        appview_save_screenshot(const AppViewState*, tools::Path); // Save an LCT_RGBA PNG image to path
    bool        appview_is_fullscreen(const AppViewState*);
    void        appview_set_fullscreen(AppViewState*, bool b );
    void        appview_set_title(AppViewState*, const char* string );
    void        appview_dock_window(AppViewState*,const char* window_name, Dockspace); // Must be called within signal_reset_layout
    void        appview_reset_layout_next_frame(AppViewState*);
    std::vector<unsigned char> appview_take_screenshot(const AppViewState*);
    
    bool        pick_file_path(tools::Path& _out_path, DialogType); // pick a file and store its path in _out_path
}