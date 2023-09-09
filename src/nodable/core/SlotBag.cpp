#include "SlotBag.h"

using namespace ndbl;

void SlotBag::apply(SlotBag::Event event, bool notify)
{
    switch (event.type)
    {
        case Event_t::CONNECT_EDGE:    return add_at( event.slot.index, event.edge, notify );
        case Event_t::DISCONNECT_EDGE: return remove_at( event.slot.index, event.edge, notify );
    }
    FW_EXPECT(false, "Unhandled case");
}

void SlotBag::remove_at(u8_t slot_id, Edge edge, bool notify)
{
    Slot& slot = m_slots[slot_id];
    std::remove(m_edges.begin(), m_edges.end(), edge);
    std::remove(slot.edges.begin(), slot.edges.end(), edge);
    if ( notify )
    {
        Event event{ Event_t::DISCONNECT_EDGE, edge, slot };
        on_remove.emit( event );
        on_change.emit( event );
    }
}

void SlotBag::add_at(u8_t slot_id, const Edge edge, bool notify)
{
    Slot& slot = m_slots.at(slot_id);
    FW_EXPECT(slot.is_full(), "Slot is full" );
    m_edges.emplace_back(edge);
    slot.edges.emplace_back(edge);
    if ( notify )
    {
        Event event{ Event_t::CONNECT_EDGE, edge, slot };
        on_add.emit( event );
        on_change.emit( event );
    }
}

bool SlotBag::allows_more(Relation relation) const
{
    FW_EXPECT(false, "TODO: check if a new edge of this type is allowed")
}

void SlotBag::set_limit(Relation relation, Way way, int i)
{
    FW_EXPECT(false, "TODO: implement!")
}

std::vector<Slot> SlotBag::by_relation(Relation relation) const
{
    std::vector<Slot> result;

    FW_EXPECT( is_primary(relation), "Not implemented for secondary relations" );
    for( size_t slot_id : m_ids_by_primary_relation[relation] )
    {
        result.push_back( m_slots[slot_id] );
    }

    return result;
}

std::vector<Edge> SlotBag::edges() const
{
    std::vector<Edge> result;

    for(const Slot& slot : m_slots )
    {
        std::copy( slot.edges.begin(), slot.edges.end(), result.end() );
    }

    return result;
}

size_t SlotBag::count(Relation relation, Way desired_way) const
{
    size_t result{0};
    for (auto index : m_ids_by_primary_relation[relation])
    {
        if( m_slots[index].allows(desired_way) )
        {
            result++;
        }
    }
    return result;
}

Slot& SlotBag::by_property(Property* property, Way way) const
{
    FW_EXPECT(false, "TODO: Implement");
}

Edge SlotBag::find_edge_at(Relation _relation, Way _desired_way, u8_t _index) const
{
    size_t count{0};
    for (auto index : m_ids_by_primary_relation[_relation])
    {
        const Slot& slot = m_slots[index];
        if( !slot.allows(_desired_way) )
        {
            continue;
        }

        if( count + slot.edges.size() < _index )
        {
            continue;
        }

        for (const auto& edge: slot.edges)
        {
            if ( count != _index)
            {
                count++;
                continue;
            }

            return edge;
        }
    }
    return Edge::null;
}
