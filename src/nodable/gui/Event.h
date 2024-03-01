#pragma once
#include <utility>

#include "FrameMode.h"
#include "SlotView.h"
#include "core/Graph.h"
#include "fw/core/Pool.h"
#include "fw/gui/EventManager.h"
#include "nodable/core/SlotRef.h"

namespace ndbl
{
    // forward declaration
    class NodeView;
    using fw::PoolID;

    enum EventID_ : fw::EventID
    {
        EventID_REQUEST_DELETE_NODE = fw::EventID_USER_DEFINED, // operation on nodes
        EventID_REQUEST_ARRANGE_HIERARCHY,
        EventID_REQUEST_SELECT_SUCCESSOR,
        EventID_REQUEST_TOGGLE_FOLDING,
        EventID_REQUEST_CREATE_NODE,
        EventID_REQUEST_CREATE_BLOCK,
        EventID_REQUEST_FRAME_SELECTION,
        EventID_REQUEST_MOVE_SELECTION,
        EventID_REQUEST_TOGGLE_ISOLATE_SELECTION,
        EventID_SLOT_DROPPED,
        EventID_SLOT_DISCONNECTED,
        EventID_NODE_SELECTION_CHANGE,
    };

    class GraphView;
    struct EventPayload_FrameNodeViews
    {
        FrameMode  mode;
        GraphView* graph_view = nullptr; // Will be deduced my the Nodable if nullptr
    };
    using Event_FrameNodeViews = fw::CustomEvent<EventID_REQUEST_FRAME_SELECTION, EventPayload_FrameNodeViews>;

    struct EventPayload_SlotPair {
        SlotRef first;
        SlotRef second;
        EventPayload_SlotPair(SlotRef&& first = {}, SlotRef&& second = {})
        : first(first)
        , second(second)
        {}
    };
    using Event_SlotDisconnected = fw::CustomEvent<EventID_SLOT_DISCONNECTED, EventPayload_SlotPair>;
    using Event_SlotDropped      = fw::CustomEvent<EventID_SLOT_DROPPED, EventPayload_SlotPair>;

    struct EventPayload_Node
    {
        PoolID<Node> node;
    };
    using Event_DeleteNode  = fw::CustomEvent<EventID_REQUEST_DELETE_NODE, EventPayload_Node>;
    using Event_ArrangeNode = fw::CustomEvent<EventID_REQUEST_ARRANGE_HIERARCHY, EventPayload_Node>;
    using Event_SelectNext  = fw::CustomEvent<EventID_REQUEST_SELECT_SUCCESSOR, EventPayload_Node>;

    enum ToggleFoldingMode
    {
        NON_RECURSIVELY = 0,
        RECURSIVELY     = 1,
    };
    struct EventPayload_ToggleFoldingEvent
    {
        ToggleFoldingMode mode;
    };
    using Event_ToggleFolding = fw::CustomEvent<EventID_REQUEST_TOGGLE_FOLDING, EventPayload_ToggleFoldingEvent>;

    struct EventPayload_NodeViewSelectionChange
    {
        PoolID<NodeView> new_selection;
        PoolID<NodeView> old_selection;
    };
    using NodeViewSelectionChangeEvent = fw::CustomEvent<EventID_NODE_SELECTION_CHANGE, EventPayload_NodeViewSelectionChange>;

    struct EventPayload_CreateNode
    {
        NodeType             node_type;            // The note type to create
        const fw::func_type* node_signature{};       // The signature of the node that must be created
        SlotView*            dragged_slot{};       // The slot view being dragged.
        Graph*               graph = nullptr;      // The graph to create the node into
        ImVec2               node_view_local_pos;  // The desired position for the new node view

        EventPayload_CreateNode(NodeType node_type, const fw::func_type* signature = nullptr )
        : node_type(node_type)
        , node_signature(signature)
        {}
    };
    using Event_CreateNode  = fw::CustomEvent<EventID_REQUEST_CREATE_NODE, EventPayload_CreateNode>;
    using Event_CreateBlock = fw::CustomEvent<EventID_REQUEST_CREATE_BLOCK, EventPayload_CreateNode>;

}// namespace ndbl
