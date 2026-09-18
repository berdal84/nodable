#pragma once

#include <SDL.h>
#include <vector>
#include <array>
#include <imgui.h>

#include "bdc/String.hpp"
#include "bdc/Types.hpp"
#include "ndbl/core/File_System.h"
#include "ndbl/core/reflection/GETTERS_SETTERS.h"
#include "Action_Manager.h"
#include "Nodable.h"
#include "Texture.h"


namespace ndbl
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

    //
    // Enum to identify dockspaces
    // 
    // -------------------------------------
    // |                TOP                |
    // |-----------------------------------|
    // |           CENTER         |  RIGHT |
    // |-----------------------------------|
    // |              BOTTOM               |
    // ------------------------------------
    //
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
        bdc::String     title;
        SDL_GLContext   sdl_gl_context                  = nullptr;
        SDL_Window*     sdl_window                      = nullptr;
        u32_t           dt_in_ms                        = 0;
        float           dt_in_s                         = 0.f;
        float           smoothed_fps                    = 0.f;
        bool            should_reset_layout             = false;
        bool            show_splashscreen               = false;
        ImGuiID         dockspaces[Dockspace_COUNT]     = {0};
        Texture*        logo                            = nullptr;
        bool            show_properties_editor          = false;
        bool            show_imgui_demo                 = false;
        bool            show_advanced_node_properties   = false;
    };

    App_View_State* appview_init();
    void            appview_shutdown();
    App_View_State* appview();
    void            appview_update();
    void            appview_draw();
    void            appview_save_screenshot(const bdc::String relative_path);
    inline void     appview_show_splashscreen(bool b) { appview()->show_splashscreen = b; }
    int             appview_fps();      // get the current frame per second (un-smoothed)
    bool            appview_is_fullscreen();
    void            appview_set_fullscreen(bool b );
    void            appview_set_title(const bdc::String& string );
    std::vector<unsigned char> appview_take_screenshot();
    
    bool            appview_pick_file_path(Path& _out_path, Dialog_Type); // pick a file and store its path in _out_path

}