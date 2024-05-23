#include "Slot.h"
#include "SlotRef.h"
#include "Node.h"

using namespace ndbl;

const Slot Slot::null{};

Slot::Slot()
: flags(SlotFlag::SlotFlag_NONE)
, m_position(0)
{
}

Slot::Slot(
    ID8<Slot>::id_t _index,
    PoolID<Node>    _node,
    SlotFlags       _flags,
    ID<Property>    _property,
    u8_t            _capacity,
    size_t          _position
    )
    : id(_index)
    , node(_node)
    , flags(_flags)
    , property(_property)
    , m_adjacent()
    , m_position(_position)
{
    ASSERT( (_flags & SlotFlag_NOT_FULL) == SlotFlag_NONE ) // cannot be set manually
    ASSERT( _capacity > 0 )
    m_adjacent.reserve(_capacity);
    flags |= SlotFlag_NOT_FULL;
}

Slot::Slot(const Slot &other)
    : id(other.id )
    , node(other.node)
    , property(other.property)
    , flags(other.flags)
    , m_adjacent(other.m_adjacent)
    , m_position(other.m_position)
{
    expand_capacity(other.capacity());
}

SlotRef Slot::first_adjacent() const
{
    if (!m_adjacent.empty())
    {
        return m_adjacent[0];
    }
    return SlotRef::null;
}

SlotRef Slot::adjacent_at(u8_t pos) const
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
    return SlotRef::null;
}

bool Slot::operator==(const Slot& other) const
{
    return node == other.node
        && type() == other.type()
        && order() == other.order()
        && property == other.property
        && id == other.id;
}

bool Slot::operator!=(const Slot& other) const
{
    return node != other.node
        || type() != other.type()
        || order() != other.order()
        || property != other.property
        || id != other.id;
}

size_t Slot::adjacent_count() const
{
    return m_adjacent.size();
}

bool Slot::is_full() const
{
    return !has_flags(SlotFlag_NOT_FULL);
}

Slot::operator bool() const
{
    return *this != null;
}

Property* Slot::get_property() const
{
    Node* _node = node.get();
    return _node ? node->get_prop_at( property ) : nullptr;
}

Node* Slot::get_node() const
{
    return node.get();
}

void Slot::add_adjacent( const SlotRef& _ref)
{
    EXPECT( _ref != *this, "Reflexive edge not handled" );
    EXPECT( type() == _ref.slot_type() , "Slot must have common type" );
    EXPECT( m_adjacent.size() < m_adjacent.capacity(), "Slot is full" );
    m_adjacent.emplace_back(_ref);
    if ( m_adjacent.size() == m_adjacent.capacity() )
    {
        flags &= ~SlotFlag_NOT_FULL; // Make sure IS_NOT_FULL is 0
    }
}

void Slot::remove_adjacent( const SlotRef& _ref )
{
    auto it = std::find( m_adjacent.begin(), m_adjacent.end(), _ref);
    EXPECT( it != m_adjacent.end(), "SlotRef not found")
    m_adjacent.erase( it );
    flags |= SlotFlag_NOT_FULL;
}

void Slot::set_flags( SlotFlags _flags)
{
    flags |= _flags;
}

SlotFlags Slot::type() const
{
    return flags & SlotFlag_TYPE_MASK;
}

SlotFlags Slot::order() const
{
    return flags & SlotFlag_ORDER_MASK;
}

bool Slot::empty() const
{
    return m_adjacent.empty();
}

const std::vector<SlotRef>& Slot::adjacent() const
{
    return m_adjacent;
}

size_t Slot::capacity() const
{
    return m_adjacent.capacity();
}

void Slot::expand_capacity( size_t _capacity )
{
    EXPECT( m_adjacent.capacity() <= _capacity, "New capacity must be strictly greater than current" );
    m_adjacent.reserve(_capacity);
    flags |= SlotFlag_NOT_FULL;
}

bool Slot::has_flags( SlotFlags _flags ) const
{
    return (flags & _flags) == _flags;
}

SlotFlags Slot::static_flags() const
{
    return flags & (SlotFlag_TYPE_MASK | SlotFlag_ORDER_MASK);
}

size_t Slot::position() const
{
    return m_position;
}
