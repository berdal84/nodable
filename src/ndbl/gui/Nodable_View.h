#pragma once

#include "gui/Nodable.h"
#include "tools/gui/Texture.h"
#include "tools/gui/App_View.h"

namespace ndbl
{
    struct App_State;

    struct App_View_State : public tools::App_View_State
    {
        tools::Texture*         logo                            = nullptr;
        bool                    show_properties_editor          = false;
        bool                    show_imgui_demo                 = false;
        bool                    show_advanced_node_properties   = false;
        bool                    scroll_to_curr_instr            = true;
        
        GETTERS_reinterpret_cast(App_State*, app, tools::App_View_State::app )
    };
    
    void nodableview_init(App_View_State*, App_State* app);
    void nodableview_deinit(App_View_State*);
    void nodableview_update(App_View_State*);
    void nodableview_draw(App_View_State*);
}