#pragma once

#include <queue>
#include "types.h"

namespace ndbl
{
    // forward declaration
    class PropertyConnector;
    class NodeConnector;
    class NodeView;

    enum class EventType
    {
        none = 0,

        delete_node_action_triggered = 0x100, // operation on nodes
        arrange_node_action_triggered,
        select_successor_node_action_triggered,
        toggle_folding_selected_node_action_triggered,
        node_view_selected,
        node_view_deselected,
        frame_all_node_views,
        frame_selected_node_views,

        property_connector_dropped = 0x200, // operation on property connectors
        property_connector_disconnected,

        node_connector_dropped   = 0x300, // operation on node connectors
        node_connector_disconnected,

        save_file_triggered = 0x400, // operation on files
        save_file_as_triggered,
        new_file_triggered,
        close_file_triggered,
        browse_file_triggered,
        undo_triggered,
        redo_triggered,
        file_opened,

        exit_triggered = 0x500, // general
        show_splashscreen_triggered,
        toggle_isolate_selection,
    };

    struct SimpleEvent
    {
        EventType type;
    };

    struct NodeViewEvent
    {
        EventType type;
        const NodeView* view;
    };

    struct ToggleFoldingEvent
    {
        EventType              type;
        bool                   recursive;
    };

    struct PropertyConnectorEvent
    {
        EventType              type;
        const PropertyConnector* src;
        const PropertyConnector* dst;
    };

    struct NodeConnectorEvent
    {
        EventType            type;
        const NodeConnector* src;
        const NodeConnector* dst;
    };

    union Event
    {
        EventType            type;
        SimpleEvent          common;
        NodeViewEvent        node;
        PropertyConnectorEvent property_connectors;
        NodeConnectorEvent   node_connectors;
        ToggleFoldingEvent   toggle_folding;
    };

    class EventManager
    {
        static std::queue<Event> s_events;
    public:
        static size_t poll_event(Event&);
        static void   push_event(Event&);
        static void   push_event(EventType); // to push a SimpleEvent by type
    };
}