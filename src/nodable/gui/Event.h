#pragma once
#include "SlotView.h"
#include "fw/core/Pool.h"
#include "fw/gui/EventManager.h"
#include "nodable/core/SlotRef.h"

namespace ndbl
{
    // forward declaration
    class NodeView;
    using fw::PoolID;

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
        EventType_slot_dropped,
        EventType_slot_disconnected,
        EventType_toggle_isolate_selection
    };

    struct NodeViewEvent
    {
        fw::EventType    type;
        PoolID<NodeView> view;
    };

    struct ToggleFoldingEvent
    {
        fw::EventType type;
        bool recursive;
    };

    struct SlotEvent
    {
        fw::EventType type;
        SlotRef       first;
        SlotRef       second;
    };

    union Event
    {
        fw::EventType          type;
        fw::Event              event;
        fw::SimpleEvent        common;
        NodeViewEvent          node;
        SlotEvent              slot;
        ToggleFoldingEvent     toggle_folding;
    };

}// namespace ndbl
