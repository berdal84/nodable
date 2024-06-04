#pragma once
#include <utility>

#include "tools/core/memory/memory.h"
#include "tools/gui/EventManager.h"

#include "ndbl/core/Graph.h"
#include "ndbl/core/SlotRef.h"

#include "Event.h"
#include "FrameMode.h"
#include "SlotView.h"

namespace ndbl
{
    // forward declaration
    class NodeView;
    using tools::PoolID;
    using tools::Vec2;
    using tools::func_type;

    enum EventID_ : tools::EventID
    {
        EventID_DELETE_NODE = tools::EventID_USER_DEFINED, // operation on nodes
        EventID_ARRANGE_NODE,
        EventID_SELECT_NEXT,
        EventID_TOGGLE_FOLDING,
        EventID_REQUEST_CREATE_NODE,
        EventID_REQUEST_CREATE_BLOCK,
        EventID_REQUEST_FRAME_SELECTION,
        EventID_MOVE_SELECTION,
        EventID_TOGGLE_ISOLATION_FLAGS,
        EventID_SLOT_DROPPED,
        EventID_SLOT_DISCONNECTED,
        EventID_SELECTION_CHANGE,
        EventID_DELETE_EDGE,
    };

    using Event_ToggleIsolationFlags = tools::Event<EventID_TOGGLE_ISOLATION_FLAGS>;
    using Event_MoveSelection        = tools::Event<EventID_MOVE_SELECTION>;

    class GraphView;
    struct EventPayload_FrameNodeViews
    {
        FrameMode  mode;
        EventPayload_FrameNodeViews(FrameMode mode)
        : mode(mode)
        {}
    };
    using Event_FrameSelection = tools::Event<EventID_REQUEST_FRAME_SELECTION, EventPayload_FrameNodeViews>;

    struct EventPayload_SlotPair {
        SlotRef first;
        SlotRef second;
        EventPayload_SlotPair(SlotRef&& first = {}, SlotRef&& second = {})
        : first(first)
        , second(second)
        {}
    };
    using Event_SlotDisconnected = tools::Event<EventID_SLOT_DISCONNECTED, EventPayload_SlotPair>;
    using Event_SlotDropped      = tools::Event<EventID_SLOT_DROPPED, EventPayload_SlotPair>;

    struct EventPayload_Node
    {
        PoolID<Node> node;
    };
    using Event_DeleteEdge  = tools::Event<EventID_DELETE_EDGE, EventPayload_SlotPair>;
    using Event_DeleteNode  = tools::Event<EventID_DELETE_NODE, EventPayload_Node>;
    using Event_ArrangeNode = tools::Event<EventID_ARRANGE_NODE, EventPayload_Node>;
    using Event_SelectNext  = tools::Event<EventID_SELECT_NEXT, EventPayload_Node>;

    enum ToggleFoldingMode
    {
        NON_RECURSIVELY = 0,
        RECURSIVELY     = 1,
    };
    struct EventPayload_ToggleFoldingEvent
    {
        ToggleFoldingMode mode;
    };
    using Event_ToggleFolding = tools::Event<EventID_TOGGLE_FOLDING, EventPayload_ToggleFoldingEvent>;

    struct EventPayload_NodeViewSelectionChange
    {
        PoolID<NodeView> new_selection;
        PoolID<NodeView> old_selection;
    };
    using Event_SelectionChange = tools::Event<EventID_SELECTION_CHANGE, EventPayload_NodeViewSelectionChange>;

    struct EventPayload_CreateNode
    {
        NodeType             node_type;                // The note type to create
        const func_type*     node_signature;           // The signature of the node that must be created
        SlotView*            dragged_slot   = nullptr; // The slot view being dragged.
        Graph*               graph          = nullptr; // The graph to create the node into
        Vec2                 node_view_local_pos;      // The desired position for the new node view

        explicit EventPayload_CreateNode(NodeType node_type )
        : node_type(node_type)
        , node_signature(nullptr)
        {}

        EventPayload_CreateNode(NodeType node_type, const tools::func_type* signature )
        : node_type(node_type)
        , node_signature(signature)
        {}
    };
    using Event_CreateNode  = tools::Event<EventID_REQUEST_CREATE_NODE, EventPayload_CreateNode>;

}// namespace ndbl
