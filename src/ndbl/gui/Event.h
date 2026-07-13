#pragma once
#include <utility>

#include "core/Event.h"
#include "ndbl/core/Graph.h"
#include "Frame_Mode.h"
#include "Node_Slot_View.h"

namespace ndbl
{
    // forward declaration
    class Node_View;
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
        Event_ID_RESET_GRAPH,
    };

    using Event_ToggleIsolationFlags = tools::Event<Event_ID_TOGGLE_ISOLATION_FLAGS>;
    using Event_MoveSelection        = tools::Event<Event_ID_MOVE_SELECTION>;

    class Graph_View;
    struct EventPayload_FrameNode_Views
    {
        Frame_Mode  mode;
        EventPayload_FrameNode_Views(Frame_Mode mode)
        : mode(mode)
        {}
    };
    using Event_FrameSelection = tools::Event<Event_ID_REQUEST_FRAME_SELECTION, EventPayload_FrameNode_Views>;

    struct EventPayload_Node_SlotPair {
        Node_Slot* first;
        Node_Slot* second;
        EventPayload_Node_SlotPair(Node_Slot* first = {}, Node_Slot* second = {})
        : first(first)
        , second(second)
        {}
    };
    using Event_Node_SlotDisconnectAll = tools::Event<Event_ID_SLOT_DISCONNECT_ALL, EventPayload_Node_SlotPair>;
    using Event_Node_SlotDropped       = tools::Event<Event_ID_SLOT_DROPPED, EventPayload_Node_SlotPair>;

    struct EventPayload_Node
    {
        Node* node;
    };
    using Event_DeleteEdge  = tools::Event<Event_ID_DELETE_EDGE, EventPayload_Node_SlotPair>;
    using Event_DeleteSelection  = tools::Event<Event_ID_DELETE_NODE, EventPayload_Node>;
    using Event_ArrangeSelection     = tools::Event<Event_ID_ARRANGE_NODE>;
    using Event_SelectNext  = tools::Event<Event_ID_SELECT_NEXT, EventPayload_Node>;

    enum ToggleFoldingMode
    {
        NON_RECURSIVELY = 0,
        RECURSIVELY     = 1,
    };
    struct EventPayload_ToggleFoldingEvent
    {
        ToggleFoldingMode mode;
    };
    using Event_ToggleFolding = tools::Event<Event_ID_TOGGLE_FOLDING, EventPayload_ToggleFoldingEvent>;

    struct EventPayload_CreateNode
    {
        Create_Node_Type_           node_type;          // The note type to create
        const Function_Descriptor*  node_signature;     // The signature of the node that must be created
        Node_Slot_View*             active_slotview;    // The slot view being dragged.
        Graph*                      graph;              // The graph to create the node into
        Vec2                        desired_screen_pos; // The desired position for the new node view

        explicit EventPayload_CreateNode(Create_Node_Type_ node_type )
        : node_type(node_type)
        , node_signature(nullptr)
        , active_slotview(nullptr)
        , graph(nullptr)
        {}

        EventPayload_CreateNode(Create_Node_Type_ node_type, const tools::Function_Descriptor* signature )
        : node_type(node_type)
        , node_signature(signature)
        {}
    };
    using Event_CreateNode  = tools::Event<Event_ID_REQUEST_CREATE_NODE, EventPayload_CreateNode>;

}// namespace ndbl
