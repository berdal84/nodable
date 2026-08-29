#pragma once

#include "bdc/Array.hpp"
#include "bdc/Types.hpp"

#include "tools/core/Flags.h"
#include "tools/core/Signals.h"

namespace ndbl
{
    // Forward declaration
    class Node;
    class Node_Property;
    class Node_View;
    struct Node_Slot_View;
    
    struct Node_Slot
    {
        // Nature of the connection allowed by a given Node_Slot
        typedef int Flags;
        enum Flag : int
        {
            //
            // primary  ---->  secondary
            //-------------------------
            // OUTPUT   ----> INPUT
            // FLOW_OUT ----> FLOW_IN
            //

            Flag_NONE          = 0,

            Flag_ORDER_1ST     = 1 << 0,
            Flag_ORDER_2ND     = 1 << 1,

            Flag_TYPE_VALUE    = 1 << 2,
            Flag_TYPE_FLOW     = 1 << 3,

            Flag_IS_INTERNAL   = 1 << 4,
            Flag_IS_FULL       = 1 << 5,

            Flag_OUTPUT        = Flag_TYPE_VALUE | Flag_ORDER_1ST,
            Flag_INPUT         = Flag_TYPE_VALUE | Flag_ORDER_2ND,
            Flag_FLOW_OUT      = Flag_TYPE_FLOW  | Flag_ORDER_1ST,
            Flag_FLOW_IN       = Flag_TYPE_FLOW  | Flag_ORDER_2ND,
            Flag_FLOW_ENTER    = Flag_FLOW_OUT   | Flag_IS_INTERNAL, // a FLOW_OUT to the INSIDE of a scope
         // Flag_FLOW_LEAVE    = Flag_FLOW_IN    | Flag_IS_BRANCH, // a FLOW_IN to the OUTSIDE of a scope

            Flag_ORDER_MASK    = Flag_ORDER_1ST  | Flag_ORDER_2ND,
            Flag_TYPE_MASK     = Flag_TYPE_FLOW  | Flag_TYPE_VALUE,
        };

        enum Event
        {
            Event_Add,
            Event_Remove
        };

        tools::Signal<void(Event, Node_Slot*)>  signal_change;
        u32_t                                   position;       // In case multiple Node_Slot exists for the same type and order, we distinguish them with their position.
        Node*                                   node;           // parent node
        Node_Property*                          property;       // parent node's property
        Node_Slot_View*                         view;
        Flags                                   flags;
        size_t                                  capacity;
        bdc::Inlined_Array<Node_Slot*, 16>      adjacent;
        
        static const Node_Slot                  null;

        Node_Slot::Flags    type() const { return this->flags & Node_Slot::Flag_TYPE_MASK;}
        Node_Slot::Flags    type_and_order() const { return this->flags & (Node_Slot::Flag_TYPE_MASK | Node_Slot::Flag_ORDER_MASK); }
        Node_Slot::Flags    order() const{return this->flags & Node_Slot::Flag_ORDER_MASK;}
        bool                is_full() const {return HAS_FLAGS(this->flags, Node_Slot::Flag_IS_FULL);}
        Node*               first_adjacent_node() const { return adjacent.size ? adjacent[0]->node : nullptr; }
        Node_Slot*          first_adjacent() const { return adjacent.size ? adjacent[0] : nullptr; }
    };

    void                    node_slot_init(Node_Slot*, Node_Slot::Flags = {}, u32_t capacity = 0 /* 0 => maxsize */, u32_t position = 0);
    Node_Slot*              node_slot_adjacent_at(const Node_Slot*, u8_t /* position */);
    inline Node_Slot::Flags node_slot_flags_toggle_order(Node_Slot::Flags flags) { return (i8_t)(flags ^ Node_Slot::Flag_ORDER_MASK); }
    void                    node_slot_add_adjacent(Node_Slot*, Node_Slot* /* other */);
    bool                    node_slot_remove_adjacent(Node_Slot*, Node_Slot* /* other */ );
}
