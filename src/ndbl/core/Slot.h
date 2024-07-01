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
    class NodeView;
    class SlotView;

    class Slot
    {
    public:
        static constexpr size_t MAX_CAPACITY = 8;
        static const Slot null;

        Slot();
        Slot(const Slot& other);
        Slot(
            Node* owner,
            SlotFlags flags,
            Property* prop = nullptr,
            size_t capacity = 1,
            size_t position = 0
        );

        Node*     get_node() const { return m_node; }
        SlotView* get_view() const { return m_view; }
        void      set_view(SlotView* view) { m_view = view; }
        Slot*     first_adjacent() const;
        Slot*     adjacent_at(u8_t ) const;
        Property* get_property() { return m_property; }
        const Property* get_property() const { return m_property; }
        size_t    adjacent_count() const;
        bool      is_full() const; // Slots have a capacity
        void      add_adjacent(Slot*);
        void      remove_adjacent(Slot*);
        void      set_flags( SlotFlags _flags );
        SlotFlags type() const;
        SlotFlags order() const;
        bool      empty() const;
        size_t    capacity() const;
        size_t    position() const;
        void      expand_capacity(size_t _capacity);
        const std::vector<Slot*>& adjacent() const;
        bool      has_flags( SlotFlags flag ) const;
        SlotFlags type_and_order() const;
        bool      is_this() const;
        SlotFlags get_flags() const;

    private:
        size_t    m_position{}; // In case multiple Slot exists for the same type and order, we distinguish them with their position.
        Node*     m_node{};     // parent node
        Property* m_property{}; // parent node's property
        SlotFlags m_flags;
        std::vector<Slot*> m_adjacent;
        SlotView* m_view{};
    };
}
