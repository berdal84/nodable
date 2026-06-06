#pragma once
#include "tools/core/Signals.h"
#include "tools/core/Containers.h"
#include "tools/core/Types.h"

namespace ndbl
{
    // Forward declaration
    class Node;
    class Node_Property;
    class Node_View;
    struct Node_Slot_View;

    // Nature of the connection allowed by a given Node_Slot
    typedef int Node_Slot_Flags;
    enum Node_Slot_Flag : int
    {
        //
        // primary  ---->  secondary
        //-------------------------
        // OUTPUT   ----> INPUT
        // FLOW_OUT ----> FLOW_IN
        //

        Node_Slot_Flag_NONE          = 0,

        Node_Slot_Flag_ORDER_1ST     = 1 << 0,
        Node_Slot_Flag_ORDER_2ND     = 1 << 1,

        Node_Slot_Flag_TYPE_VALUE    = 1 << 2,
        Node_Slot_Flag_TYPE_FLOW     = 1 << 3,

        Node_Slot_Flag_IS_INTERNAL   = 1 << 4,
        Node_Slot_Flag_IS_FULL       = 1 << 5,

        Node_Slot_Flag_OUTPUT        = Node_Slot_Flag_TYPE_VALUE | Node_Slot_Flag_ORDER_1ST,
        Node_Slot_Flag_INPUT         = Node_Slot_Flag_TYPE_VALUE | Node_Slot_Flag_ORDER_2ND,
        Node_Slot_Flag_FLOW_OUT      = Node_Slot_Flag_TYPE_FLOW  | Node_Slot_Flag_ORDER_1ST,
        Node_Slot_Flag_FLOW_IN       = Node_Slot_Flag_TYPE_FLOW  | Node_Slot_Flag_ORDER_2ND,
        Node_Slot_Flag_FLOW_ENTER    = Node_Slot_Flag_FLOW_OUT | Node_Slot_Flag_IS_INTERNAL, // a FLOW_OUT to the INSIDE of a scope
        // Node_Slot_Flag_FLOW_LEAVE  = Node_Slot_Flag_FLOW_IN    | Node_Slot_Flag_IS_BRANCH, // a FLOW_IN to the OUTSIDE of a scope

        Node_Slot_Flag_ORDER_MASK    = Node_Slot_Flag_ORDER_1ST  | Node_Slot_Flag_ORDER_2ND,
        Node_Slot_Flag_TYPE_MASK     = Node_Slot_Flag_TYPE_FLOW  | Node_Slot_Flag_TYPE_VALUE,
    };

    static Node_Slot_Flags switch_order(Node_Slot_Flags flags)
    { return (i8_t)(flags ^ Node_Slot_Flag_ORDER_MASK); }

    struct Node_Slot
    {
        Node_Slot(
            Node_Slot_Flags flags    = {},
            size_t    capacity = 0, // 0 => max
            size_t    position = 0
        );

        enum Event
        {
            Event_Add,
            Event_Remove
        };

        tools::Array_View<Node_Slot*> adjacent() { return _adjacent;}
        tools::Array_View<const Node_Slot*> adjacent() const { return _adjacent;}
        Node_Slot*      adjacent_at(u8_t) const;
        size_t          adjacent_count() const { return _adjacent.size; }
        Node*           first_adjacent_node() const { return !_adjacent.empty() ? _adjacent[0]->node : nullptr; }
        Node_Slot*      first_adjacent() const { return !_adjacent.empty() ? _adjacent[0] : nullptr; }
        Node_Slot_Flags flags() const { return _flags; }
        void            set_flags( Node_Slot_Flags flags){_flags |= flags;}
        bool            has_flags( Node_Slot_Flags flags ) const{return (_flags & flags) == flags;}
        Node_Slot_Flags type() const{return _flags & Node_Slot_Flag_TYPE_MASK;}
        Node_Slot_Flags type_and_order() const { return _flags & (Node_Slot_Flag_TYPE_MASK | Node_Slot_Flag_ORDER_MASK); }
        Node_Slot_Flags order() const{return _flags & Node_Slot_Flag_ORDER_MASK;}
        bool            empty() const{return _adjacent.empty();}
        size_t          capacity() const{return _capacity; }
        bool            is_full() const {return has_flags(Node_Slot_Flag_IS_FULL);}
        void            add_adjacent(Node_Slot*);
        bool            remove_adjacent(Node_Slot*);

        tools::Signal<void(Event, Node_Slot*)>  signal_change;
        const size_t                            position        = {}; // In case multiple Node_Slot exists for the same type and order, we distinguish them with their position.
        Node*                                   node            = {}; // parent node
        Node_Property*                          property        = {}; // parent node's property
        Node_Slot_View*                         view            = {};
        
        Node_Slot_Flags                         _flags          = {};
        size_t                                  _capacity       = {};
        tools::Inline_Vector16<Node_Slot*>      _adjacent;
        
        static const Node_Slot                  null;
    };
}
