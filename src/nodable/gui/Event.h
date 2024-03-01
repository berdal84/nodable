#pragma once
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
        EventID_REQUEST_FRAME_ALL,
        EventID_REQUEST_CREATE_NODE,
        EventID_REQUEST_CREATE_BLOCK,
        EventID_REQUEST_FRAME_SELECTION,
        EventID_REQUEST_TOGGLE_ISOLATE_SELECTION,
        EventID_SLOT_DROPPED,
        EventID_SLOT_DISCONNECTED,
        EventID_NODE_SELECTION_CHANGE,
    };

    struct NodeViewSelectionChangePayload
    {
        PoolID<NodeView> new_selection;
        PoolID<NodeView> old_selection;
    };
    using NodeViewSelectionChangeEvent = fw::TEvent<EventID_NODE_SELECTION_CHANGE, NodeViewSelectionChangePayload>;

    struct ToggleFoldingEventPayload
    {
        bool recursive = false;
    };
    using ToggleFoldingEvent = fw::TEvent<EventID_REQUEST_TOGGLE_FOLDING, ToggleFoldingEventPayload>;

    struct SlotEventPayload
    {
        SlotRef       first;
        SlotRef       second;
    };
    using SlotDisconnectedEvent = fw::TEvent<EventID_SLOT_DISCONNECTED, SlotEventPayload>;
    using SlotDroppedEvent      = fw::TEvent<EventID_SLOT_DROPPED, SlotEventPayload>;

    struct CreateNodeEventPayload
    {
        NodeType             node_type;            // The note type to create
        const fw::func_type* node_signature;       // The signature of the node that must be created
        SlotView*            dragged_slot;         // The slot view being dragged.
        Graph*               graph = nullptr;      // The graph to create the node into
        ImVec2               node_view_local_pos;  // The desired position for the new node view
    };
    using CreateNodeEvent  = fw::TEvent<EventID_REQUEST_CREATE_NODE, CreateNodeEventPayload>;
    using CreateBlockEvent = fw::TEvent<EventID_REQUEST_CREATE_BLOCK, CreateNodeEventPayload>;
}// namespace ndbl
