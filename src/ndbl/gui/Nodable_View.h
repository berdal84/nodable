#pragma once

#include "core/reflection/GETTERS_SETTERS.h"
#include "gui/Nodable.h"
#include "tools/gui/Texture.h"
#include "tools/gui/App_View.h"

namespace ndbl
{
    struct App_State;

    struct App_View_State 
    {
        tools::App_View_State   base;
        tools::Texture*         logo                            = nullptr;
        bool                    show_properties_editor          = false;
        bool                    show_imgui_demo                 = false;
        bool                    show_advanced_node_properties   = false;
        bool                    scroll_to_curr_instr            = true;
        
        SETTER(bool, show_splashscreen, base.show_splashscreen);
    };

    App_View_State* appview_init();
    void            appview_shutdown();
    App_View_State* appview();
    void            appview_update();
    void            appview_draw();
    void            appview_save_screenshot(const bdc::String relative_path);
    inline void     appview_show_splashscreen(bool b) { appview()->base.show_splashscreen = b; }

}