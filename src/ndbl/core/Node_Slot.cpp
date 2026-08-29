#include "Node_Slot.h"
#include "Node.h"
#include "core/Flags.h"

namespace ndbl
{
using namespace bdc;
using namespace tools;

const Node_Slot Node_Slot::null{};

void node_slot_init(
    Node_Slot*          slot,
    Node_Slot::Flags    flags,
    u32_t               capacity,
    u32_t               position
)
{
    VERIFY( !HAS_FLAGS(flags, Node_Slot::Flag_IS_FULL), "Node_Slot::Flag_IS_FULL is for readonly use" );
    
    if (capacity == 0)
    {
        capacity = slot->adjacent.capacity();
    }

    ASSERT( capacity <= slot->adjacent.capacity() );

    slot->position = position;
    slot->capacity = capacity;
    slot->flags    = flags;
}

Node_Slot* node_slot_adjacent_at(const Node_Slot* slot, u8_t pos)
{
    ASSERT(pos < slot->capacity);    
    if ( pos < slot->adjacent.size)
        return slot->adjacent[pos];
    return nullptr;
}

void node_slot_add_adjacent(Node_Slot* slot, Node_Slot* other)
{
    ASSERT(other != nullptr);
    VERIFY(other != slot, "Reflexive edge not handled" );
    VERIFY(slot->type() == other->type() , "Node_Slot must have common type" );
    VERIFY(slot->adjacent.size < slot->capacity, "Capacity max reached!" );
    array_append(slot->adjacent, other);
    if ( slot->adjacent.size == slot->capacity )
    {
        slot->flags |= Node_Slot::Flag_IS_FULL; // make sure IS_FULL is 1
    }
    slot->signal_change.emit(Node_Slot::Event_Add, other);
}

bool node_slot_remove_adjacent(Node_Slot* slot, Node_Slot* other)
{
    Array_Find_Result result = array_find(slot->adjacent, other);
    if( !result.found )
    {
        TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Node_Slot", "remove_adjacent(Node_Slot*) - slot not found");
        return false;
    }
    array_remove_ordered(slot->adjacent, result.at_pos );
    slot->flags &= ~Node_Slot::Flag_IS_FULL; // make sure IS_FULL is 0
    slot->signal_change.emit(Node_Slot::Event_Remove, other);
    return true;
}
} // namespace ndbl
