#pragma once
#include "fw/gui/EventManager.h"
#include "fw/core/Pool.h"

namespace ndbl
{
    // forward declaration
    class PropertyConnector;
    class NodeConnector;
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
        EventType_property_connector_dropped,                                // operation on property connectors
        EventType_property_connector_disconnected,
        EventType_node_connector_dropped,                                    // operation on node connectors
        EventType_node_connector_disconnected,
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

    struct ConnectorEvent {
        fw::EventType      type;
        union {
            const NodeConnector*     node;
            const PropertyConnector* prop;
        } src;
        union {
            const NodeConnector*     node;
            const PropertyConnector* prop;
        } dst;
    };

    union Event
    {
        fw::EventType          type;
        fw::Event              event;
        fw::SimpleEvent        common;
        NodeViewEvent          node;
        ConnectorEvent         connector;
        ToggleFoldingEvent     toggle_folding;
    };

}// namespace ndbl
