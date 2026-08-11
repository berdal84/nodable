#pragma once

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

    typedef int Event_Code;
    enum Event_Code_
    {
        Event_Code_DELETE,
        Event_Code_ARRANGE_SELECTION,
        Event_Code_SELECT_NEXT,
        Event_Code_TOGGLE_FOLDING,
        Event_Code_REQUEST_CREATE_NODE,
        Event_Code_REQUEST_CREATE_BLOCK,
        Event_Code_REQUEST_FRAME_SELECTION,
        Event_Code_MOVE_SELECTION,
        Event_Code_TOGGLE_ISOLATION_FLAGS,
        Event_Code_SLOT_DROPPED,
        Event_Code_DELETE_ALL_LINKS,
        Event_Code_SELECTION_CHANGE,
        Event_Code_DELETE_LINK,
        Event_Code_RESET_GRAPH_VIEW,
    };

    struct Event_Data__Selection
    {
        View_Selection selected_items = {};
    };

    struct Event_Data__Create_Node
    {
        // TODO: Remove active_slotview and desired_screen_pos, they can be obtained from the app state
        //       This way we can avoid to have this struct and use the Event_User_Data struct instead perhaps?

        Create_Node_Type            node_type           = Create_Node_Type_NULL;    // The note type to create
        const Function_Descriptor*  node_signature      = nullptr;                  // The signature of the node that must be created
        Node_Slot_View*             active_slotview     = nullptr;                  // The slot view being dragged.
        Vec2                        desired_screen_pos  = {};                       // The desired position for the new node view

        Event_Data__Create_Node(Create_Node_Type _node_type, const Function_Descriptor* _node_signature = nullptr)
        : node_type(_node_type)
        , node_signature(_node_signature)
        {}
    };

}// namespace ndbl
