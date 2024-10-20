#include "Slot.h"
#include "Node.h"

using namespace ndbl;

const Slot Slot::null{};

Slot::Slot()
: _flags(SlotFlag::SlotFlag_NONE)
{
}

Slot::Slot(
    Node*     node,
    SlotFlags flags,
    Property* property,
    size_t    capacity,
    size_t    position
    )
: _node(node)
, _flags(flags)
, _property(property)
, _position(position)
{
    VERIFY(!has_flags(SlotFlag_NOT_FULL), "SlotFlag_NOT_FULL is for readonly use" )
    ASSERT( capacity > 0 )
    _adjacent.reserve(capacity);
    _flags |= SlotFlag_NOT_FULL;
}

Slot::Slot(const Slot &other)
: _node(other._node)
, _property(other._property)
, _flags(other._flags)
, _adjacent(other._adjacent)
, _position(other._position)
{
    expand_capacity(other.capacity());
}

Slot* Slot::first_adjacent() const
{
    if (!_adjacent.empty())
    {
        return _adjacent[0];
    }
    return nullptr;
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

size_t Slot::adjacent_count() const
{
    return _adjacent.size();
}

bool Slot::is_full() const
{
    return !has_flags(SlotFlag_NOT_FULL);
}

void Slot::_add_adjacent(Slot* other)
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
}

void Slot::_remove_adjacent(Slot* other)
{
    auto it = std::find(_adjacent.begin(), _adjacent.end(), other);
    VERIFY(it != _adjacent.end(), "Slot* not found")
    _adjacent.erase(it );
    _flags |= SlotFlag_NOT_FULL;
}

void Slot::set_flags( SlotFlags flags)
{
    _flags |= flags;
}

SlotFlags Slot::type() const
{
    return _flags & SlotFlag_TYPE_MASK;
}

SlotFlags Slot::order() const
{
    return _flags & SlotFlag_ORDER_MASK;
}

bool Slot::empty() const
{
    return _adjacent.empty();
}

const std::vector<Slot*>& Slot::adjacent() const
{
    return _adjacent;
}

size_t Slot::capacity() const
{
    return _adjacent.capacity();
}

void Slot::expand_capacity( size_t capacity )
{
    VERIFY(_adjacent.capacity() <= capacity, "New capacity must be strictly greater than current" );
    _adjacent.reserve(capacity);
    _flags |= SlotFlag_NOT_FULL;
}

bool Slot::has_flags( SlotFlags flags ) const
{
    return (_flags & flags) == flags;
}

SlotFlags Slot::type_and_order() const
{
    return _flags & (SlotFlag_TYPE_MASK | SlotFlag_ORDER_MASK);
}

size_t Slot::position() const
{
    return _position;
}

SlotFlags Slot::flags() const
{
    return _flags;
}

void Slot::connect_bidirectionally(Slot* tail, Slot* head)
{
    ASSERT( tail != head)
    ASSERT( head != nullptr )
    ASSERT( tail != nullptr )

    tail->_add_adjacent(head);
    head->_add_adjacent(tail);

    if ( head->_node != nullptr )
        head->_node->set_adjacent_cache_dirty();

    if ( tail->_node != nullptr )
        tail->_node->set_adjacent_cache_dirty();
}
