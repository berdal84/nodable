#pragma once

#include <cstring>

#include "bdc/String.hpp"
#include "bdc/Types.hpp"
#include "bdc/Types.hpp"
#include "geometry/Vec2.h"
#include "Graph.h"
#include "reflection/Type_Descriptor.h"
#include "View.h"
#include "Node.h"

namespace ndbl
{
    // forward declaration
    struct Node_View;
    struct Node_Slot;
    struct Node;
    struct Graph;

    typedef u32_t Event_Type;
    enum Event_Type_ : Event_Type
    {
        Event_Type_NULL = 0,

        // file-related event types
        Event_Type_FILE_SAVE                    = 1,
        Event_Type_FILE_SAVE_AS                 = 2,
        Event_Type_FILE_NEW                     = 3,
        Event_Type_FILE_CLOSE                   = 4,
        Event_Type_FILE_BROWSE                  = 5,
        Event_Type_FILE_OPENED                  = 6,

        // history-related events
        Event_Type_UNDO                         = 100,
        Event_Type_REDO                         = 101,                        

        // common events
        Event_Type_REQUEST_EXIT                 = 200,
        Event_Type_TOGGLE_HELP                  = 201,
        
        // nodable-specific events
        Event_Type_RESET_LAYOUT                 = 300, 
        Event_Type_DELETE_ALL_LINKS             = 301,
        Event_Type_DELETE_LINK                  = 302,
        Event_Type_DELETE                       = 303,
        Event_Type_MOVE                         = 304,
        Event_Type_NEW_NODE                     = 305,
        Event_Type_FRAME_SELECTION              = 306,
        Event_Type_RESET_GRAPH_VIEW             = 307,
        Event_Type_SELECT_NEXT                  = 308,
        Event_Type_SELECTION_CHANGE             = 309,
        Event_Type_SLOT_DROPPED_ONTO_ANOTHER    = 310,
        Event_Type_TOGGLE_FOLDING               = 311,
        Event_Type_TOGGLE_ISOLATION_FLAGS       = 312,
    };

    struct Event_Data__Window
    {
        const bdc::String imgui_id;
    };

    typedef u16_t Event_User_Code;

    struct Event
    {
        Event_Type      type;
        Event_User_Code code;
        void*           data1;
        void*           data2;

        operator bool ()
        { return type != Event_Type_NULL; }
    };
    
    static constexpr Event null_event{};

    struct Event_Data__Selection
    {
        View_Selection selected_items = {};
    };

    struct Event_Data__Create_Node
    {
        Node_State          node_state;
        Node_Slot_View*     active_slotview;    // The slot view being dragged.
        Vec2                desired_screen_pos; // The desired position for the new node view
        Type_Descriptor*    function_type;
    };

}