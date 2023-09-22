#pragma once
#include <observe/event.h>
#include <set>
#include <map>
#include <vector>

#include "DirectedEdge.h"
#include "Slot.h"
#include "fw/core/assertions.h"

namespace ndbl
{
    struct SlotBag
    {
        enum Event_t
        {
            CONNECT_EDGE,
            DISCONNECT_EDGE
        };

        struct Event
        {
            Event_t      type;
            SlotRef      slot;
            SlotRef      adjacent;
        };

        observe::Event<Event> on_change;
        observe::Event<Event> on_add;    // same as on_change but only emit ADD events
        observe::Event<Event> on_remove; // same as on_change but only emit REMOVE events

        SlotBag(){}

        std::vector<Slot>& data()
        { return m_slots; }

        const std::vector<Slot>& data() const
        { return m_slots; }

        Slot& back()
        { return m_slots.back(); }

        size_t size() const
        { return m_slots.size(); }

        Slot& operator[](size_t _index)
        { return m_slots[_index]; }

        const Slot& operator[](size_t _index) const
        { return m_slots[_index]; }

        ID8<Slot>         add(PoolID<Node> _node, ID<Property> _prop_id, SlotFlags _flags, u8_t _capacity = SLOT_MAX_CAPACITY);
        void              apply(Event, bool notify = true);
        void              set_capacity(ID8<Slot>, int i);
        size_t            count(SlotFlags) const;
        Slot&             by_property( ID<Property> property, SlotFlags);
        const Slot&       by_property( ID<Property> property, SlotFlags) const;
        Slot*             find_adjacent_at( SlotFlags, u8_t _index ) const;
        std::vector<Slot*> filter( SlotFlags ) const;
    private:
        const Slot&       _by_property( ID<Property> property, SlotFlags ) const;
        void              remove_edge_at(ID8<Slot> id, SlotRef, bool notify = true);
        void              add_adjacent_at(ID8<Slot> id, SlotRef, bool notify = true);

    private:
        std::vector<Slot> m_slots;
    };
}