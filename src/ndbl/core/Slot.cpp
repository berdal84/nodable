#include "Slot.h"
#include "Node.h"

using namespace ndbl;

const Slot Slot::null{};

Slot::Slot()
: node(nullptr)
, _flags(SlotFlag_NONE)
, property(nullptr)
, position(0)
{
}

Slot::Slot(
    Node*     node,
    SlotFlags flags,
    Property* property,
    size_t    capacity,
    size_t    position
    )
: node(node)
, _flags(flags)
, property(property)
, position(position)
{
    VERIFY(!has_flags(SlotFlag_NOT_FULL), "SlotFlag_NOT_FULL is for readonly use" );
    ASSERT( capacity > 0 );
    _adjacent.reserve(capacity);
    _flags |= SlotFlag_NOT_FULL;
}

Slot::Slot(const Slot &other)
: node(other.node)
, property(other.property)
, _flags(other._flags)
, _adjacent(other._adjacent)
, position(other.position)
{
    expand_capacity(other.capacity());
}

Slot* Slot::adjacent_at(u8_t pos) const
{
    u8_t count{0};
    for (auto& each : _adjacent )
    {
        if( count == pos )
        {
            return each;
        }
        ++count;
    }
    return nullptr;
}

void Slot::add_adjacent(Slot* other)
{
    ASSERT(other != nullptr);
    VERIFY(other != this, "Reflexive edge not handled" );
    VERIFY(type() == other->type() , "Slot must have common type" );
    VERIFY(_adjacent.size() < _adjacent.capacity(), "Slot is full" );
    _adjacent.emplace_back( other );
    if (_adjacent.size() == _adjacent.capacity() )
    {
        _flags &= ~SlotFlag_NOT_FULL; // Make sure IS_NOT_FULL is 0
    }
    on_change.emit(Event_Add, other);
}

void Slot::remove_adjacent(Slot* other)
{
    auto it = std::find(_adjacent.begin(), _adjacent.end(), other);
    VERIFY(it != _adjacent.end(), "Slot* not found");
    _adjacent.erase(it );
    _flags |= SlotFlag_NOT_FULL;
    on_change.emit(Event_Remove, other);
}

void Slot::expand_capacity( size_t capacity )
{
    VERIFY(_adjacent.capacity() <= capacity, "New capacity must be strictly greater than current" );
    _adjacent.reserve(capacity);
    _flags |= SlotFlag_NOT_FULL;
}

