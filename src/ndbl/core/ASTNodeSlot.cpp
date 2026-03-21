#include "ASTNodeSlot.h"
#include "ASTNode.h"

using namespace ndbl;

const ASTNodeSlot ASTNodeSlot::null{};

ASTNodeSlot::ASTNodeSlot(
    SlotFlags flags,
    size_t    capacity,
    size_t    position
)
: _capacity(capacity)
, _flags(flags)
, position(position)
{
    VERIFY(!has_flags(SlotFlag_IS_FULL), "SlotFlag_IS_FULL is for readonly use" );
    if (_capacity == 0)
        _capacity = _adjacent.capacity();
    ASSERT( _capacity <= _adjacent.capacity() );
}

ASTNodeSlot* ASTNodeSlot::adjacent_at(u8_t pos) const
{
    ASSERT(pos < _capacity);    
    if ( pos < _adjacent.size)
        return _adjacent[pos];
    return nullptr;
}

void ASTNodeSlot::add_adjacent(ASTNodeSlot* other)
{
    ASSERT(other != nullptr);
    VERIFY(other != this, "Reflexive edge not handled" );
    VERIFY(type() == other->type() , "Slot must have common type" );
    VERIFY(_adjacent.size < _capacity, "Capacity max reached!" );
    _adjacent.push_back( other );
    if ( _adjacent.size == _capacity )
    {
        _flags |= SlotFlag_IS_FULL; // make sure IS_FULL is 1
    }
    signal_change.emit(Event_Add, other);
}

bool ASTNodeSlot::remove_adjacent(ASTNodeSlot* other)
{
    auto it = std::find(_adjacent.begin(), _adjacent.end(), other);
    if( it == _adjacent.end())
    {
        TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "ASTNodeSlot", "remove_adjacent(Slot*) - slot not found");
        return false;
    }
    _adjacent.erase(it );
    _flags &= ~SlotFlag_IS_FULL; // make sure IS_FULL is 0
    signal_change.emit(Event_Remove, other);
    return true;
}
