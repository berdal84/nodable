#pragma once
#include <observe/event.h>
#include <set>
#include <vector>

#include "Edge.h"
#include "Relation.h"
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
            Event_t type;
            Edge    edge;
            Slot    slot;
        };

        SlotBag(){}

        std::vector<Slot> data()
        { return m_slots; }

        const std::vector<Slot> data()const
        { return m_slots; }

        Slot back()
        { return m_slots.back(); }

        size_t size() const
        { return m_slots.size(); }

        Slot operator[](size_t _index) const
        { return m_slots[_index]; }

        void              apply(Event, bool notify = true);
        void              remove_at(u8_t id, Edge, bool notify = true);
        void              add_at(u8_t it, Edge, bool notify = true );
        bool              allows_more(Relation) const;
        void              set_limit(Relation, Way, int i);
        std::vector<Slot> by_relation(Relation) const;
        std::vector<Edge> edges() const;
        size_t            count(Relation, Way) const;
        Slot&             by_property(Property*, Way) const;
        Edge              find_edge_at(Relation, Way /* desired_way */, u8_t _index) const;
        
        observe::Event<Event> on_change;
        observe::Event<Event> on_add;    // same as on_change but only emit ADD events
        observe::Event<Event> on_remove; // same as on_change but only emit REMOVE events
    private:
        std::vector<Slot> m_slots;
        std::vector<Edge> m_edges;
        std::array<std::vector<u8_t>, Relation::PRIMARY_COUNT> m_ids_by_primary_relation; // edge type to slot ids
    };
}