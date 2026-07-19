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
        GETTERS_reinterpret_cast(App_State*, app, base.app )
    };
    
    void        nodableview_init(App_View_State*, App_State* app);
    void        nodableview_deinit(App_View_State*);
    void        nodableview_update(App_View_State*);
    void        nodableview_draw(App_View_State*);
    void        nodableview_save_screenshot(const App_View_State*, const char* relative_path);

}