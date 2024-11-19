#pragma once
#include "SlotFlag.h"
#include "tools/core/Signals.h"
#include "tools/core/memory/memory.h"
#include "tools/core/types.h"
#include <vector>

namespace ndbl
{
    // Forward declaration
    class Node;
    class Property;
    class NodeView;
    struct SlotView;

    struct Slot
    {
        Slot();
        Slot(const Slot& other);
        Slot(
            Node*     owner,
            SlotFlags flags,
            Property* prop = nullptr,
            size_t    capacity = 1,
            size_t    position = 0
        );

        enum Event
        {
            Event_Add,
            Event_Remove
        };

        // assign your own delegate once here, it will be called when this Slot changes
        SIGNAL(on_change, Event, Slot*);

        Slot*             adjacent_at(u8_t) const;
        inline size_t     adjacent_count() const{return _adjacent.size();}
        inline const std::vector<Slot*>&
                          adjacent() const{return _adjacent;}
        inline Node*      first_adjacent_node() const { return !_adjacent.empty() ? _adjacent[0]->node : nullptr; }
        inline Slot*      first_adjacent() const { return !_adjacent.empty() ? _adjacent[0] : nullptr; }
        void              expand_capacity(size_t _capacity);
        inline SlotFlags  flags() const { return _flags; }
        inline void       set_flags( SlotFlags flags){_flags |= flags;}
        inline bool       has_flags( SlotFlags flags ) const{return (_flags & flags) == flags;}
        inline SlotFlags  type() const{return _flags & SlotFlag_TYPE_MASK;}
        inline SlotFlags  type_and_order() const { return _flags & (SlotFlag_TYPE_MASK | SlotFlag_ORDER_MASK); }
        inline SlotFlags  order() const{return _flags & SlotFlag_ORDER_MASK;}
        inline bool       empty() const{return _adjacent.empty();}
        inline size_t     capacity() const{return _adjacent.capacity(); }
        inline bool       is_full() const {return !has_flags(SlotFlag_NOT_FULL);}
        void              add_adjacent(Slot*);
        void              remove_adjacent(Slot*);

        const size_t     position; // In case multiple Slot exists for the same type and order, we distinguish them with their position.
        Node* const      node; // parent node
        Property* const  property; // parent node's property
        SlotFlags        _flags = SlotFlag_NONE;
        SlotView*        view = nullptr;
        std::vector<Slot*> _adjacent;

        static constexpr size_t MAX_CAPACITY = 8;
        static const     Slot   null;
    };
}
