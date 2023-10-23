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
        size_t            count(SlotFlags) const;
        Slot*             find_by_property( ID<Property>, SlotFlags );
        const Slot*       find_by_property( ID<Property>, SlotFlags ) const;
        Slot*             find_adjacent_at( SlotFlags, u8_t _index ) const;
        std::vector<Slot*> filter( SlotFlags ) const;
    private:
        std::vector<Slot> m_slots;
    };
}