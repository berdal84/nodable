#include "DirectedEdge.h"

#include <utility>
#include "Slot.h"
#include "Node.h"

using namespace ndbl;

DirectedEdge::DirectedEdge(Slot* _tail, Slot* _head )
: tail(_tail)
, head(_head)
{
    ASSERT(tail->flags() & SlotFlag_ORDER_FIRST );
    ASSERT(head->flags() & SlotFlag_ORDER_SECOND );
    ASSERT(tail->node->graph() != nullptr);
    ASSERT(head->node->graph() != nullptr);
    ASSERT(tail->node->graph() == head->node->graph() );
}

std::string ndbl::to_string(const DirectedEdge& _edge)
{
    std::string result;
    result.reserve(64);

    auto serialize_slot_ref = [&result](const Slot* _slot) -> void
    {
        result.append("[node: ");
        result.append( std::to_string((u64_t) _slot->node));
        result.append(" (slot: ");
        result.append( std::to_string((u64_t)_slot));

        switch (_slot->flags() )
        {
            case SlotFlag_CHILD:   result.append(", CHILD");  break;
            case SlotFlag_PARENT:  result.append(", PARENT"); break;
            case SlotFlag_INPUT:   result.append(", INPUT");  break;
            case SlotFlag_OUTPUT:  result.append(", OUTPUT"); break;
        }

        result.append(")]");
    };

    serialize_slot_ref(_edge.tail);

    auto type = _edge.tail->flags() & SlotFlag_TYPE_MASK;
    // TODO: enable reflection on SLotFlag_XXX
    switch ( type )
    {
        case SlotFlag_TYPE_VALUE:
            result.append(" >==(VALUE)==> ");
            break;
        case SlotFlag_TYPE_CODEFLOW:
            result.append(" >==(CODEFLOW)==> ");
            break;
        case SlotFlag_TYPE_HIERARCHICAL:
            result.append(" >==(HIERARCHY)==> ");
            break;
        default:
            ASSERT(false); // unhandled type?
    }

    serialize_slot_ref(_edge.head);


    return std::move(result);
}

