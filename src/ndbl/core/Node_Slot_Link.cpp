#include "Node_Slot_Link.h"

#include <utility>
#include "Node_Slot.h"
#include "Node.h"

using namespace ndbl;

Node_Slot_Link::Node_Slot_Link(Node_Slot* _tail, Node_Slot* _head )
: tail(_tail)
, head(_head)
{
    // Guards
    ASSERT(tail);
    ASSERT(head);
    VERIFY(tail->type() == head->type(), "Node_Slot types are incompatible"  );
    VERIFY(tail->node != head->node    , "Can't connect two slots from the same node");

    if ( tail->order() == Node_Slot::Flag_ORDER_2ND ) // Make sure tail is always FIRST ORDER
        std::swap(tail, head);

    ASSERT(tail->flags & Node_Slot::Flag_ORDER_1ST );
    ASSERT(head->flags & Node_Slot::Flag_ORDER_2ND );
    ASSERT(tail->node->graph != nullptr);
    ASSERT(head->node->graph != nullptr);
    VERIFY(tail->node->graph == head->node->graph,"The slots are from Nodes from different graphs" );
}

std::string ndbl::to_string(const Node_Slot_Link& _edge)
{
    std::string result;
    result.reserve(64);

    auto serialize_slot_ref = [&result](const Node_Slot* _slot) -> void
    {
        result.append("[node: ");
        result.append( std::to_string((u64_t) _slot->node));
        result.append(" (slot: ");
        result.append( std::to_string((u64_t)_slot));

        switch (_slot->flags)
        {
            case Node_Slot::Flag_INPUT:   result.append(", INPUT");  break;
            case Node_Slot::Flag_OUTPUT:  result.append(", OUTPUT"); break;
        }

        result.append(")]");
    };

    serialize_slot_ref(_edge.tail);

    // TODO: enable reflection on SLotFlag_XXX
    switch ( _edge.tail->type() )
    {
        case Node_Slot::Flag_TYPE_VALUE:
            result.append(" >==(VALUE)==> ");
            break;
        case Node_Slot::Flag_TYPE_FLOW:
            result.append(" >==(CODEFLOW)==> ");
            break;
        default:
            ASSERT(false); // unhandled type?
    }

    serialize_slot_ref(_edge.head);

    return std::move(result);
}

