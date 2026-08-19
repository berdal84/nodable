#include "Node_Slot.h"
#include "Node.h"
#include "core/Flags.h"

using namespace ndbl;

const Node_Slot Node_Slot::null{};

void ndbl::node_slot_init(
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

Node_Slot* ndbl::node_slot_adjacent_at(const Node_Slot* slot, u8_t pos)
{
    ASSERT(pos < slot->capacity);    
    if ( pos < slot->adjacent.size)
        return slot->adjacent[pos];
    return nullptr;
}

void ndbl::node_slot_add_adjacent(Node_Slot* slot, Node_Slot* other)
{
    ASSERT(other != nullptr);
    VERIFY(other != slot, "Reflexive edge not handled" );
    VERIFY(slot->type() == other->type() , "Node_Slot must have common type" );
    VERIFY(slot->adjacent.size < slot->capacity, "Capacity max reached!" );
    slot->adjacent.push_back( other );
    if ( slot->adjacent.size == slot->capacity )
    {
        slot->flags |= Node_Slot::Flag_IS_FULL; // make sure IS_FULL is 1
    }
    slot->signal_change.emit(Node_Slot::Event_Add, other);
}

bool ndbl::node_slot_remove_adjacent(Node_Slot* slot, Node_Slot* other)
{
    auto it = std::find(slot->adjacent.begin(), slot->adjacent.end(), other);
    if( it == slot->adjacent.end())
    {
        TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Node_Slot", "remove_adjacent(Node_Slot*) - slot not found");
        return false;
    }
    slot->adjacent.erase(it );
    slot->flags &= ~Node_Slot::Flag_IS_FULL; // make sure IS_FULL is 0
    slot->signal_change.emit(Node_Slot::Event_Remove, other);
    return true;
}
