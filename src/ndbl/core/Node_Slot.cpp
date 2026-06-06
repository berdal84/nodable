#include "Node_Slot.h"
#include "Node.h"

using namespace ndbl;

const Node_Slot Node_Slot::null{};

Node_Slot::Node_Slot(
    Node_Slot_Flags flags,
    size_t    capacity,
    size_t    position
)
: _capacity(capacity)
, _flags(flags)
, position(position)
{
    VERIFY(!has_flags(Node_Slot_Flag_IS_FULL), "Node_Slot_Flag_IS_FULL is for readonly use" );
    if (_capacity == 0)
        _capacity = _adjacent.capacity();
    ASSERT( _capacity <= _adjacent.capacity() );
}

Node_Slot* Node_Slot::adjacent_at(u8_t pos) const
{
    ASSERT(pos < _capacity);    
    if ( pos < _adjacent.size)
        return _adjacent[pos];
    return nullptr;
}

void Node_Slot::add_adjacent(Node_Slot* other)
{
    ASSERT(other != nullptr);
    VERIFY(other != this, "Reflexive edge not handled" );
    VERIFY(type() == other->type() , "Node_Slot must have common type" );
    VERIFY(_adjacent.size < _capacity, "Capacity max reached!" );
    _adjacent.push_back( other );
    if ( _adjacent.size == _capacity )
    {
        _flags |= Node_Slot_Flag_IS_FULL; // make sure IS_FULL is 1
    }
    signal_change.emit(Event_Add, other);
}

bool Node_Slot::remove_adjacent(Node_Slot* other)
{
    auto it = std::find(_adjacent.begin(), _adjacent.end(), other);
    if( it == _adjacent.end())
    {
        TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Node_Slot", "remove_adjacent(Node_Slot*) - slot not found");
        return false;
    }
    _adjacent.erase(it );
    _flags &= ~Node_Slot_Flag_IS_FULL; // make sure IS_FULL is 0
    signal_change.emit(Event_Remove, other);
    return true;
}
