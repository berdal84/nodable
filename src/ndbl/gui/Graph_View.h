#pragma once

#include "core/Types.h"
#include "ndbl/gui/View.h"
#include "tools/core/reflection/GETTERS_SETTERS.h"
#include "tools/core/Component.h"  // base class

#include "Node_View.h"
#include "tools/core/State_Machine.h"
#include "Node_Search_Input.h"

namespace ndbl
{
    // forward declarations
    class  Nodable;
    class  Graph;
    struct Node_View_Constraint;
    using  tools::Vec2;

    typedef u8_t Graph_View_Flags;
    enum Graph_View_Flag: u8_t
    {
        Graph_View_Flag_NONE                    = 0,
        Graph_View_Flag_NEEDS_TO_BE_RESET       = 1 << 0,
        Graph_View_Flag_NEEDS_TO_FRAME_CONTENT  = 1 << 1
    };

    struct Graph_View : public tools::Component<Graph>
    {
	    Graph_View();
		~Graph_View();

        Graph_View_Flags                    flags = 0;
        tools::Simple_Signal                signal_change;
        Node_Search_Input                   node_search_input;
        View                                hovered;
        View                                focused;
        View_Selection                      selection;
        tools::Box_2D                       shape;
        tools::State_Machine                state_machine;
        tools::Vec2                         state_roi_start_pos;
        tools::Vec2                         state_roi_end_pos;   
        
        GETTER(Graph*, graph, entity);
    };

    void    graphview_update(Graph_View*, float dt);
    bool    graphview_draw(Graph_View*, float dt);
    bool    graphview_has_an_active_tool(const Graph_View*);
    void    graphview_reset_all_properties(Graph_View*);
}