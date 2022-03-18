#pragma once

#include <queue>
#include "types.h"

namespace Nodable
{
    // forward declaration
    class MemberConnector;
    class NodeConnector;

    enum class EventType
    {
        delete_node_triggered = 0x100, // operation on nodes
        arrange_node_triggered,
        select_successor_node_triggered,
        expand_selected_node_triggered,

        member_connector_dropped = 0x200, // operation on member connectors
        member_connector_disconnected,

        node_connector_dropped   = 0x300, // operation on node connectors
        node_connector_disconnected
    };

    struct SimpleEvent
    {
        EventType type;
    };

    struct MemberConnectorEvent
    {
        EventType              type;
        const MemberConnector* src;
        const MemberConnector* dst;
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
        MemberConnectorEvent member_connectors;
        NodeConnectorEvent   node_connectors;
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