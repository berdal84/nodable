#pragma once

#include <queue>
#include "nodable/Nodable.h"

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

        member_connector_dropped_on_another = 0x200, // operation on connectors
        node_connector_dropped_on_another
    };

    struct Event_Simple
    {
        EventType type;
    };

    struct Event_MemberConnectorLinked
    {
        EventType              type;
        const MemberConnector* src;
        const MemberConnector* dst;
    };

    struct Event_NodeConnectorLinked
    {
        EventType            type;
        const NodeConnector* src;
        const NodeConnector* dst;
    };

    union Event
    {
        EventType     type;
        Event_Simple  common;
        Event_MemberConnectorLinked member_connectors;
        Event_NodeConnectorLinked   node_connectors;
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