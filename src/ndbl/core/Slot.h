#pragma once
#include "SlotFlag.h"
#include "tools/core/memory/memory.h"
#include "tools/core/types.h"
#include <vector>

namespace ndbl
{
    // Forward declaration
    class Node;
    class Property;
    class SlotRef;
    class NodeView;
    class SlotView;
    using tools::PoolID;
    using tools::ID;
    using tools::ID8;

    constexpr u8_t SLOT_MAX_CAPACITY = std::numeric_limits<u8_t>::max();

    class Slot
    {
    public:
        static const Slot null;

        ID8<Slot>         id;         // slot index (in node's SlotBag)
        PoolID<Node>      node;       // node id (in tools::Pool)
        ID<Property>      property;   // property index (in node's PropertyBag)
        SlotFlags         flags;

        Slot();
        Slot(const Slot& other);
        Slot(
            ID8<Slot>::id_t id,
            PoolID<Node> owner,
            SlotFlags flags,
            ID<Property> prop = {},
            u8_t capacity = 1,
            size_t position = 0
        );

        bool operator==(const Slot&) const;
        bool operator!=(const Slot&) const;
        explicit operator bool () const;

        Node*     get_node() const;
        SlotView* get_view() const { return m_view; }
        void      set_view(SlotView* view) { m_view = view; }
        SlotRef   first_adjacent() const;
        SlotRef   adjacent_at(u8_t ) const;
        Property* get_property() const; // Dereference the property corresponding to this Slot.
        size_t    adjacent_count() const;
        bool      is_full() const; // Slots have a capacity
        void      add_adjacent( const SlotRef& );
        void      remove_adjacent( const SlotRef& );
        void      set_flags( SlotFlags _flags );
        SlotFlags type() const;
        SlotFlags order() const;
        bool      empty() const;
        size_t    capacity() const;
        size_t    position() const;
        void      expand_capacity(size_t _capacity);
        const std::vector<SlotRef>& adjacent() const;
        bool      has_flags( SlotFlags flag ) const;

        SlotFlags type_and_order() const;

        bool is_this() const;

    private:
        size_t               m_position;   // In case multiple Slot exists for the same type and order, we distinguish them with their position.
        std::vector<SlotRef> m_adjacent;
        SlotView*            m_view;
    };
}
