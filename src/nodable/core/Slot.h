#pragma once
#include "SlotFlag.h"
#include "fw/core/types.h"
#include "fw/core/Pool.h"
#include <vector>

namespace ndbl
{
    // Forward declaration
    class Node;
    class Property;
    class SlotRef;
    using fw::PoolID;
    using fw::ID;
    using fw::ID8;

    constexpr u8_t SLOT_MAX_CAPACITY = std::numeric_limits<u8_t>::max();

    class Slot
    {
    public:
        static const Slot null;

        ID8<Slot>         id;         // slot index (in node's SlotBag)
        PoolID<Node>      node;       // node id (in fw::Pool)
        ID<Property>      property;   // property index (in node's PropertyBag)
        SlotFlags         flags;

        Slot();
        Slot(const Slot& other);
        Slot(ID8<Slot>::id_t, PoolID<Node>, SlotFlags, ID<Property> = {}, u8_t capacity = 0);

        bool operator==(const Slot&) const;
        bool operator!=(const Slot&) const;
        explicit operator bool () const;

        Node*     get_node() const;
        SlotRef   first_adjacent() const;
        SlotRef   adjacent_at(u8_t ) const;
        Property* get_property() const; // Dereference the property corresponding to this Slot.
        size_t    adjacent_count() const;
        bool      is_full() const; // Slots have a capacity
        void      add_adjacent( const SlotRef& );
        void      remove_adjacent( const SlotRef& );
        void      allow( SlotFlags );
        SlotFlags type() const;
        SlotFlags order() const;
        bool      empty() const;
        size_t    capacity() const;
        void      expand_capacity(size_t _capacity);
        const std::vector<SlotRef>& adjacent() const;
    private:
        std::vector<SlotRef> m_adjacent;
    };
}
