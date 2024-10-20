#include "Slot.h"
#include "Node.h"

using namespace ndbl;

const Slot Slot::null{};

Slot::Slot()
: m_flags(SlotFlag::SlotFlag_NONE)
{
}

Slot::Slot(
    Node*     _node,
    SlotFlags _flags,
    Property* _property,
    size_t    _capacity,
    size_t    _position
    )
: m_node(_node)
, m_flags(_flags)
, m_property(_property)
, m_position(_position)
{
    VERIFY(!has_flags(SlotFlag_NOT_FULL), "SlotFlag_NOT_FULL is for readonly use" )
    ASSERT( _capacity > 0 )
    m_adjacent.reserve(_capacity);
    m_flags |= SlotFlag_NOT_FULL;
}

Slot::Slot(const Slot &other)
: m_node(other.m_node)
, m_property(other.m_property)
, m_flags(other.m_flags)
, m_adjacent(other.m_adjacent)
, m_position(other.m_position)
{
    expand_capacity(other.capacity());
}

Slot* Slot::first_adjacent() const
{
    if (!m_adjacent.empty())
    {
        return m_adjacent[0];
    }
    return nullptr;
}

Slot* Slot::adjacent_at(u8_t pos) const
{
    u8_t count{0};
    for (auto& each : m_adjacent )
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
    return m_adjacent.size();
}

bool Slot::is_full() const
{
    return !has_flags(SlotFlag_NOT_FULL);
}

void Slot::add_adjacent(Slot* _other)
{
    ASSERT(_other != nullptr);
    VERIFY(_other != this, "Reflexive edge not handled" );
    VERIFY(type() == _other->type() , "Slot must have common type" );
    VERIFY(m_adjacent.size() < m_adjacent.capacity(), "Slot is full" );
    m_adjacent.emplace_back(_other);
    if ( m_adjacent.size() == m_adjacent.capacity() )
    {
        m_flags &= ~SlotFlag_NOT_FULL; // Make sure IS_NOT_FULL is 0
    }
}

void Slot::remove_adjacent(Slot* _other)
{
    auto it = std::find(m_adjacent.begin(), m_adjacent.end(), _other);
    VERIFY(it != m_adjacent.end(), "Slot* not found")
    m_adjacent.erase( it );
    m_flags |= SlotFlag_NOT_FULL;
}

void Slot::set_flags( SlotFlags _flags)
{
    m_flags |= _flags;
}

SlotFlags Slot::type() const
{
    return m_flags & SlotFlag_TYPE_MASK;
}

SlotFlags Slot::order() const
{
    return m_flags & SlotFlag_ORDER_MASK;
}

bool Slot::empty() const
{
    return m_adjacent.empty();
}

const std::vector<Slot*>& Slot::adjacent() const
{
    return m_adjacent;
}

size_t Slot::capacity() const
{
    return m_adjacent.capacity();
}

void Slot::expand_capacity( size_t _capacity )
{
    VERIFY(m_adjacent.capacity() <= _capacity, "New capacity must be strictly greater than current" );
    m_adjacent.reserve(_capacity);
    m_flags |= SlotFlag_NOT_FULL;
}

bool Slot::has_flags( SlotFlags _flags ) const
{
    return (m_flags & _flags) == _flags;
}

SlotFlags Slot::type_and_order() const
{
    return m_flags & (SlotFlag_TYPE_MASK | SlotFlag_ORDER_MASK);
}

size_t Slot::position() const
{
    return m_position;
}

SlotFlags Slot::flags() const
{
    return m_flags;
}
