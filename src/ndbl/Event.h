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

        // basic events (no data)
        Event_Type_FILE_SAVE,
        Event_Type_FILE_SAVE_AS,
        Event_Type_FILE_NEW,
        Event_Type_FILE_CLOSE,
        Event_Type_FILE_BROWSE,
        Event_Type_FILE_OPENED,
        Event_Type_UNDO,
        Event_Type_REDO,
        Event_Type_REQUEST_EXIT,
        Event_Type_TOGGLE_HELP,

        // nodable specifics
        Event_Type_RESET_LAYOUT,
        Event_Type_DELETE_ALL_LINKS,
        Event_Type_DELETE_LINK,
        Event_Type_DELETE,
        Event_Type_MOVE,
        Event_Type_NEW_NODE,
        Event_Type_FRAME_SELECTION,
        Event_Type_RESET_GRAPH_VIEW,
        Event_Type_SELECT_NEXT,
        Event_Type_SELECTION_CHANGE,
        Event_Type_SLOT_DROPPED_ONTO_ANOTHER,
        Event_Type_TOGGLE_FOLDING,
        Event_Type_TOGGLE_ISOLATION_FLAGS,
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