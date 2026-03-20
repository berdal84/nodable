#pragma once
#include "ASTNodeSlotFlag.h"
#include "tools/core/Signals.h"
#include "tools/core/Containers.h"

#include <vector>

namespace ndbl
{
    // Forward declaration
    class ASTNode;
    class ASTNodeProperty;
    class ASTNodeView;
    struct ASTNodeSlotView;

    struct ASTNodeSlot
    {
        ASTNodeSlot(
            SlotFlags flags    = {},
            size_t    capacity = 0, // 0 => max
            size_t    position = 0
        );

        enum Event
        {
            Event_Add,
            Event_Remove
        };

        ASTNodeSlot* adjacent_at(u8_t) const;
        size_t       adjacent_count() const { return _adjacent.size; }
        tools::ArrayView<ASTNodeSlot*> adjacent() { return _adjacent;}
        tools::ArrayView<const ASTNodeSlot*> adjacent() const { return _adjacent;}
        ASTNode*     first_adjacent_node() const { return !_adjacent.empty() ? _adjacent[0]->node : nullptr; }
        ASTNodeSlot* first_adjacent() const { return !_adjacent.empty() ? _adjacent[0] : nullptr; }
        SlotFlags    flags() const { return _flags; }
        void         set_flags( SlotFlags flags){_flags |= flags;}
        bool         has_flags( SlotFlags flags ) const{return (_flags & flags) == flags;}
        SlotFlags    type() const{return _flags & SlotFlag_TYPE_MASK;}
        SlotFlags    type_and_order() const { return _flags & (SlotFlag_TYPE_MASK | SlotFlag_ORDER_MASK); }
        SlotFlags    order() const{return _flags & SlotFlag_ORDER_MASK;}
        bool         empty() const{return _adjacent.empty();}
        size_t       capacity() const{return _capacity; }
        bool         is_full() const {return !has_flags(SlotFlag_NOT_FULL);}
        void         add_adjacent(ASTNodeSlot*);
        bool         remove_adjacent(ASTNodeSlot*);

        tools::Signal<void(Event, ASTNodeSlot*)>    signal_change;
        const size_t                                position        = {}; // In case multiple Slot exists for the same type and order, we distinguish them with their position.
        ASTNode*                                    node            = {}; // parent node
        ASTNodeProperty*                            property        = {}; // parent node's property
        ASTNodeSlotView*                            view            = {};
        
        SlotFlags                                   _flags          = {};
        size_t                                      _capacity       = {};
        tools::InlineVector16<ASTNodeSlot*>         _adjacent;
        
        static const ASTNodeSlot                    null;
    };
}
