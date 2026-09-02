#pragma once

#include <SDL.h>
#include <vector>
#include <array>
#include "gui/Action_Manager.h"
#include "imgui.h"
#include "tools/core/File_System.h"
#include "bdc/String.hpp"
#include "bdc/Types.hpp"
#include "tools/core/Signals.h"

namespace tools
{
    // forward declarations
    class App_State;    
    class Texture_Manager;
    class Event_Manager;
    class Font_Manager;

    enum Dialog_Type // Helps to configure the file browse dialog
    {
        Dialog_Type_SaveAs,   // Allows to set a new file or select an existing file
        Dialog_Type_Browse    // Only allows to pick a file
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

	struct App_View_State
	{
        tools::Simple_Signal signal_reset_layout; // add custom code during layout reset
        tools::Simple_Signal signal_draw_splashscreen_content; // to insert custom code into the splashscreen window

        bdc::String         title;
        SDL_GLContext       sdl_gl_context   = nullptr;
        SDL_Window*         sdl_window       = nullptr;
        u32_t               dt_in_ms         = 0;
        float               dt_in_s          = 0.f;
        float               smoothed_fps     = 0.f;
        bool                should_reset_layout = false;
        bool                show_splashscreen = false;
        ImGuiID             dockspaces[Dockspace_COUNT] = {0};
    };

    void        appview_init(App_View_State*, App_State*);
    void        appview_update(App_View_State*);
    void        appview_begin(App_View_State*);
    void        appview_end(App_View_State*);
    void        appview_deinit(App_View_State*);
    void        appview_draw_splashscreen(App_View_State*); // If needed, use begin/end_splashscreen static methods to override this. Ex: if ( AppView::begin_splashscreen(m_app->config) ) { /* your code here */; AppView::end_splashscreen(); }
    ImGuiID     appview_get_dockspace(App_View_State*, Dockspace);
    void        appview_dock_window(App_View_State*, const bdc::String& window_name, Dockspace); // Must be called within signal_reset_layout
    int         appview_fps(App_View_State*);      // get the current frame per second (un-smoothed)
    void        appview_save_screenshot(const App_View_State*, const tools::Path& ); // Save an LCT_RGBA PNG image to path
    bool        appview_is_fullscreen(const App_View_State*);
    void        appview_set_fullscreen(App_View_State*, bool b );
    void        appview_set_title(App_View_State*, const bdc::String& string );
    std::vector<unsigned char> appview_take_screenshot(const App_View_State*);
    
    bool        pick_file_path(tools::Path& _out_path, Dialog_Type); // pick a file and store its path in _out_path
}