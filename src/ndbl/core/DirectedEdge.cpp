#include "DirectedEdge.h"
#include "Slot.h"
#include "Node.h"

using namespace ndbl;

DirectedEdge DirectedEdge::null{};

DirectedEdge::DirectedEdge()// is equivalent of NULL DirectedEdge
{}

DirectedEdge::DirectedEdge( const DirectedEdge &other )
: tail(other.tail)
, head(other.head)
{
}

DirectedEdge::DirectedEdge( SlotRef _tail, SlotRef _head )
: tail(_tail)
, head(_head)
{
    ASSERT(_tail.flags & SlotFlag_ORDER_FIRST )
    ASSERT(_head.flags & SlotFlag_ORDER_SECOND )
}

bool DirectedEdge::operator!=( const DirectedEdge &other ) const
{
    return this->tail != other.tail || this->head == other.head;
}

DirectedEdge &DirectedEdge::operator=( const DirectedEdge& other )
{
    tail     = other.tail;
    head     = other.head;
    return *this;
}

bool DirectedEdge::operator==( const DirectedEdge &other ) const
{
    return this->tail == other.tail && this->head == other.head;
}

DirectedEdge::operator bool() const
{
    return *this != null;
}

std::string ndbl::to_string(const DirectedEdge& _edge)
{
    std::string result;
    result.reserve(64);

    auto serialize_slot_ref = [&result](const SlotRef& _slot_ref) -> void
    {
        result.append("[node: ");
        result.append( std::to_string((u64_t )_slot_ref.node.id) );
        result.append(" (slot: ");
        result.append( std::to_string((u8_t)_slot_ref.id));

        switch ( _slot_ref.flags )
            {
                case SlotFlag_CHILD:   result.append(", CHILD");  break;
                case SlotFlag_PARENT:  result.append(", PARENT"); break;
                case SlotFlag_INPUT:   result.append(", INPUT");  break;
                case SlotFlag_OUTPUT:  result.append(", OUTPUT"); break;
            }

        result.append(")]");
    };

    serialize_slot_ref(_edge.tail);

    auto type = _edge.tail.flags & SlotFlag_TYPE_MASK;
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
    }

    serialize_slot_ref(_edge.head);


    return std::move(result);
}

