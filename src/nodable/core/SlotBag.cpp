#include "SlotBag.h"
#include "Node.h"

using namespace ndbl;

void SlotBag::apply(SlotBag::Event event, bool notify)
{
    switch (event.type)
    {
        case Event_t::CONNECT_EDGE:    return add_adjacent_at(event.slot.id, event.adjacent, notify );
        case Event_t::DISCONNECT_EDGE: return remove_edge_at(event.slot.id, event.adjacent, notify);
    }
    FW_EXPECT(false, "Unhandled case");
}

void SlotBag::remove_edge_at(ID8<Slot> slot_id, SlotRef adjacent, bool notify)
{
    Slot& slot = m_slots[slot_id];
    std::remove(slot.adjacent.begin(), slot.adjacent.end(), adjacent);
    if ( notify )
    {
        Event event;
        event.type     = Event_t::DISCONNECT_EDGE;
        event.slot     = slot;
        event.adjacent = adjacent;
        on_remove.emit( event );
        on_change.emit( event );
    }
}

void SlotBag::add_adjacent_at(ID8<Slot> id, SlotRef adjacent, bool notify)
{
    Slot& slot = m_slots.at( id );
    FW_EXPECT(slot.is_full(), "Slot is full" );
    slot.adjacent.emplace_back( adjacent );
    if ( notify )
    {
        Event event;
        event.type     = Event_t::CONNECT_EDGE;
        event.slot     = slot;
        event.adjacent = adjacent;

        on_add.emit( event );
        on_change.emit( event );
    }
}

void SlotBag::set_capacity( ID8<Slot> _id, int _capacity )
{
    m_slots[_id].set_capacity(_capacity);
}

size_t SlotBag::count(SlotFlags flags) const
{
   return filter(flags).size();
}

Slot* SlotBag::find_by_property(ID<Property> property_id, SlotFlags _flags)
{
    return const_cast<Slot*>( _find_by_property( property_id, _flags ) );
}

const Slot* SlotBag::find_by_property(ID<Property> property_id, SlotFlags flags) const
{
    return _find_by_property( property_id, flags );
}

const Slot* SlotBag::_find_by_property(ID<Property> property_id, SlotFlags _flags) const
{
    for(auto& slot : m_slots )
    {
        if( (slot.flags & _flags) == _flags && slot.property == property_id )
        {
            return &slot;
        }
    }
    return nullptr;
}

Slot* SlotBag::find_adjacent_at( SlotFlags flags, u8_t _index ) const
{
    size_t count{0};
    for (auto& slot : m_slots)
    {
        if( (slot & flags) == flags )
        {
            continue;
        }

        if( count + slot.adjacent.size() < _index )
        {
            count += slot.adjacent.size();
            continue;
        }

        for (const auto& edge: slot.adjacent )
        {
            if ( count == _index)
            {
                return edge.get();
            }
            count++;
        }
    }
    return nullptr;
}

ID8<Slot> SlotBag::add(PoolID<Node> _node, ID<Property> _prop_id, SlotFlags _flags, u8_t _capacity)
{
    FW_EXPECT(_node != PoolID<Node>::null, "node cannot be null");

    size_t id = m_slots.size();
    FW_ASSERT(id < std::numeric_limits<u8_t>::max() )
    Slot& slot = m_slots.emplace_back( (u8_t)id, _node, _flags, _prop_id, _capacity);

    return slot.id;
}

std::vector<Slot*> SlotBag::filter( SlotFlags _flags ) const
{
    std::vector<Slot*> result;
    for(auto& slot : m_slots)
    {
        if( (slot.flags & _flags) == _flags )
        {
            result.push_back(const_cast<Slot*>( &slot ));
        }
    }
    return result;
}
