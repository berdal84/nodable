#pragma once

#include "core/Graph.h"
#include "ndbl/gui/View.h"
#include "tools/core/reflection/Type_Descriptor.h"
#include "tools/core/Event.h"
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

    enum Event_ID_ : tools::Event_ID
    {
        Event_ID_DELETE_NODE = tools::Event_ID_USER_DEFINED, // operation on nodes
        Event_ID_ARRANGE_NODE,
        Event_ID_SELECT_NEXT,
        Event_ID_TOGGLE_FOLDING,
        Event_ID_REQUEST_CREATE_NODE,
        Event_ID_REQUEST_CREATE_BLOCK,
        Event_ID_REQUEST_FRAME_SELECTION,
        Event_ID_MOVE_SELECTION,
        Event_ID_TOGGLE_ISOLATION_FLAGS,
        Event_ID_SLOT_DROPPED,
        Event_ID_SLOT_DISCONNECT_ALL,
        Event_ID_SELECTION_CHANGE,
        Event_ID_DELETE_EDGE,
        Event_ID_RESET_GRAPH_VIEW,
    };

    using Event_ToggleIsolationFlags = tools::Event<Event_ID_TOGGLE_ISOLATION_FLAGS>;
    using Event_MoveSelection        = tools::Event<Event_ID_MOVE_SELECTION>;
    using Event_FrameSelection       = tools::Event<Event_ID_REQUEST_FRAME_SELECTION>;

    struct EventPayload_Node_SlotPair
    {
        Node_Slot* first    = nullptr;
        Node_Slot* second   = nullptr;
    };

    using Event_Node_SlotDisconnectAll = tools::Event<Event_ID_SLOT_DISCONNECT_ALL, EventPayload_Node_SlotPair>;
    using Event_Node_SlotDropped       = tools::Event<Event_ID_SLOT_DROPPED, EventPayload_Node_SlotPair>;

    struct EventPayload_Node
    {
        Node* node;
    };
    using Event_DeleteEdge          = tools::Event<Event_ID_DELETE_EDGE, EventPayload_Node_SlotPair>;

    struct EventPayload_Selection
    {
        View_Selection selected_items = {};
    };
    using Event_DeleteSelection     = tools::Event<Event_ID_DELETE_NODE     , EventPayload_Selection>;
    using Event_ArrangeSelection    = tools::Event<Event_ID_ARRANGE_NODE    , EventPayload_Selection>;
    using Event_SelectNext          = tools::Event<Event_ID_SELECT_NEXT     , EventPayload_Selection>;
    using Event_ToggleFolding       = tools::Event<Event_ID_TOGGLE_FOLDING  , EventPayload_Selection>;

    struct EventPayload_CreateNode
    {
        Create_Node_Type            node_type           = Create_Node_Type_NULL;    // The note type to create
        const Function_Descriptor*  node_signature      = nullptr;                  // The signature of the node that must be created
        Node_Slot_View*             active_slotview     = nullptr;                  // The slot view being dragged.
        Graph*                      graph               = nullptr;                  // The graph to create the node into
        Vec2                        desired_screen_pos  = {};                       // The desired position for the new node view
    };
    using Event_CreateNode = tools::Event<Event_ID_REQUEST_CREATE_NODE, EventPayload_CreateNode>;

}// namespace ndbl
