#pragma once
#include <utility>

#include "Event.h"
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
        EventID_DELETE_NODE = fw::EventID_USER_DEFINED, // operation on nodes
        EventID_ARRANGE_NODE,
        EventID_SELECT_NEXT,
        EventID_TOGGLE_FOLDING,
        EventID_REQUEST_CREATE_NODE,
        EventID_REQUEST_CREATE_BLOCK,
        EventID_REQUEST_FRAME_SELECTION,
        EventID_MOVE_SELECTION,
        EventID_TOGGLE_ISOLATE,
        EventID_SLOT_DROPPED,
        EventID_SLOT_DISCONNECTED,
        EventID_SELECTION_CHANGE,
    };

    using Event_ToggleIsolate   = fw::Event<EventID_TOGGLE_ISOLATE>;
    using Event_MoveSelection   = fw::Event<EventID_MOVE_SELECTION>;

    class GraphView;
    struct EventPayload_FrameNodeViews
    {
        FrameMode  mode;
        EventPayload_FrameNodeViews(FrameMode mode)
        : mode(mode)
        {}
    };
    using Event_FrameSelection = fw::Event<EventID_REQUEST_FRAME_SELECTION, EventPayload_FrameNodeViews>;

    struct EventPayload_SlotPair {
        SlotRef first;
        SlotRef second;
        EventPayload_SlotPair(SlotRef&& first = {}, SlotRef&& second = {})
        : first(first)
        , second(second)
        {}
    };
    using Event_SlotDisconnected = fw::Event<EventID_SLOT_DISCONNECTED, EventPayload_SlotPair>;
    using Event_SlotDropped      = fw::Event<EventID_SLOT_DROPPED, EventPayload_SlotPair>;

    struct EventPayload_Node
    {
        PoolID<Node> node;
    };
    using Event_DeleteNode  = fw::Event<EventID_DELETE_NODE, EventPayload_Node>;
    using Event_ArrangeNode = fw::Event<EventID_ARRANGE_NODE, EventPayload_Node>;
    using Event_SelectNext  = fw::Event<EventID_SELECT_NEXT, EventPayload_Node>;

    enum ToggleFoldingMode
    {
        NON_RECURSIVELY = 0,
        RECURSIVELY     = 1,
    };
    struct EventPayload_ToggleFoldingEvent
    {
        ToggleFoldingMode mode;
    };
    using Event_ToggleFolding = fw::Event<EventID_TOGGLE_FOLDING, EventPayload_ToggleFoldingEvent>;

    struct EventPayload_NodeViewSelectionChange
    {
        PoolID<NodeView> new_selection;
        PoolID<NodeView> old_selection;
    };
    using Event_SelectionChange = fw::Event<EventID_SELECTION_CHANGE, EventPayload_NodeViewSelectionChange>;

    struct EventPayload_CreateNode
    {
        NodeType             node_type;                // The note type to create
        const fw::func_type* node_signature;           // The signature of the node that must be created
        SlotView*            dragged_slot   = nullptr; // The slot view being dragged.
        Graph*               graph          = nullptr; // The graph to create the node into
        fw::Vec2 node_view_local_pos;      // The desired position for the new node view

        explicit EventPayload_CreateNode(NodeType node_type )
        : node_type(node_type)
        , node_signature(nullptr)
        {}

        EventPayload_CreateNode(NodeType node_type, const fw::func_type* signature )
        : node_type(node_type)
        , node_signature(signature)
        {}
    };
    using Event_CreateNode  = fw::Event<EventID_REQUEST_CREATE_NODE, EventPayload_CreateNode>;

}// namespace ndbl
