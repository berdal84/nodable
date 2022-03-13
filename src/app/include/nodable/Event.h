#pragma once

#include <queue>
#include "nodable/Nodable.h"

namespace Nodable
{
    // forward declaration
    class Member;

    enum class EventType
    {
        delete_selected_node = 0x000, // operation on nodes
        arrange_selected_node,
        select_successor_node,
        expand_selected_node,

        connect_members = 0x100 // operation on members
    };

    struct Event_Simple
    {
        EventType type;
    };

    struct Event_ConnectMembers
    {
        EventType    type;
        Member* src;
        Member* dst;
        ConnBy_ conn_by;
    };

    union Event
    {
        EventType                 type;
        Event_Simple              common;
        Event_ConnectMembers      connect_members;
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