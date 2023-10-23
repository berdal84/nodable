#include "SlotBag.h"
#include "Node.h"

using namespace ndbl;

size_t SlotBag::count(SlotFlags flags) const
{
   return filter(flags).size();
}

Slot* SlotBag::find_by_property(ID<Property> property_id, SlotFlags _flags)
{
    return const_cast<Slot*>(  const_cast<const SlotBag*>(this)->find_by_property( property_id, _flags ) );
}

const Slot* SlotBag::find_by_property(ID<Property> property_id, SlotFlags _flags) const
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
    size_t cursor_pos{0};
    for (auto& slot : m_slots)
    {
        // Skip any slot not compatible with given flags
        if( (slot.flags & flags) != flags )
        {
            continue;
        }

        // if the position is in the range of this slot, we return the item
        size_t local_pos = (size_t)_index - cursor_pos;
        if ( local_pos < slot.adjacent_count() )
        {
            return slot.adjacent_at(local_pos).get();
        }
        // increase counter
        cursor_pos += slot.adjacent_count();
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
