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
    class SlotView;

    struct Slot
    {
        enum Event
        {
            Event_Add,
            Event_Remove
        };

        static constexpr size_t MAX_CAPACITY = 8;
        static const     Slot   null;

        Slot();
        Slot(const Slot& other);
        Slot(
            Node*     owner,
            SlotFlags flags,
            Property* prop = nullptr,
            size_t    capacity = 1,
            size_t    position = 0
        );

        inline Node* node() const { return _node; }
        inline SlotView* view() const { return _view; }
        inline const Property* property() const { return _property; }
        void      set_view(SlotView* view) { _view = view; }
        Slot*     first_adjacent() const;
        Slot*     adjacent_at(u8_t ) const;
        Property* get_property() { return _property; }
        size_t    adjacent_count() const;
        bool      is_full() const; // Slots have a capacity
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
        SlotFlags flags() const;

        static void connect_bidirectionally(Slot* tail, Slot* head);
        static void disconnect_bidirectionally(Slot* tail, Slot* head);

        void      _add_adjacent(Slot*);
        void      _remove_adjacent(Slot*);

        // assign your own delegate once here, it will be called when this Slot changes
        SIGNAL(on_change, Event, Slot*);

        size_t    _position = 0; // In case multiple Slot exists for the same type and order, we distinguish them with their position.
        Node*     _node     = nullptr; // parent node
        Property* _property = nullptr; // parent node's property
        SlotFlags _flags    = SlotFlag_NONE;
        SlotView* _view     = nullptr;
        std::vector<Slot*> _adjacent;
    };
}
