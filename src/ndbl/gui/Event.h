#pragma once

#include "core/Event.h"
#include "core/Graph.h"
#include "core/Types.h"
#include "ndbl/gui/View.h"
#include "tools/core/reflection/Type_Descriptor.h"
#include "tools/gui/geometry/Vec2.h"

namespace ndbl
{
    // forward declaration
    struct Node_View;
    struct Node_Slot;
    struct Node;
    struct Graph;
    using tools::Vec2;
    using tools::Function_Descriptor;

    typedef tools::Event_Type Event_Type; // alias under different namespace
    enum Event_Type_ : tools::Event_Type
    {
        Event_Type_RESET_LAYOUT = tools::Event_Type_USER, // we must start from this value to make sure we're not using existing values from tools::Event_Type
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

    struct Event_Data__Selection
    {
        View_Selection selected_items = {};
    };

    struct Event_Data__Create_Node
    {
        Node_State              node_state;
        Node_Slot_View*         active_slotview;    // The slot view being dragged.
        Vec2                    desired_screen_pos; // The desired position for the new node view
        Function_Descriptor*    function_descriptor;
    };

}// namespace ndbl
