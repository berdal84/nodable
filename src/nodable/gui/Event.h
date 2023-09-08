#pragma once
#include "SlotView.h"
#include "fw/core/Pool.h"
#include "fw/gui/EventManager.h"

namespace ndbl
{
    // forward declaration
    class PropertyConnectorView;
    class NodeView;
    using fw::pool::ID;

    enum EventType_: fw::EventType
    {
        EventType_delete_node_action_triggered = fw::EventType_USER_DEFINED, // operation on nodes
        EventType_arrange_node_action_triggered,
        EventType_select_successor_node_action_triggered,
        EventType_toggle_folding_selected_node_action_triggered,
        EventType_node_view_selected,
        EventType_node_view_deselected,
        EventType_frame_all_node_views,
        EventType_frame_selected_node_views,
        EventType_connector_dropped,
        EventType_connector_disconnected,
        EventType_toggle_isolate_selection
    };

    struct NodeViewEvent {
        fw::EventType          type;
        ID<const NodeView> view;
    };

    struct ToggleFoldingEvent {
        fw::EventType type;
        bool recursive;
    };

    struct SlotEvent
    {
        fw::EventType type;
        Slot          first;
        Slot          second;
    };

    union Event
    {
        fw::EventType          type;
        fw::Event              event;
        fw::SimpleEvent        common;
        NodeViewEvent          node;
        SlotEvent connector;
        ToggleFoldingEvent     toggle_folding;
    };

}// namespace ndbl
