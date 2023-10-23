#include "Slot.h"
#include "SlotRef.h"
#include "Node.h"

using namespace ndbl;

const Slot Slot::null{};

Slot::Slot()
: flags(SlotFlag::SlotFlag_NONE)
, capacity(0)
{}

Slot::Slot(const Slot &other)
: id(other.id )
, node(other.node)
, property(other.property)
, flags(other.flags)
, capacity(other.capacity)
, m_adjacent(other.m_adjacent )
{
}


Slot::Slot(
ID8<Slot>::id_t _index,
PoolID<Node> _node,
SlotFlags    _flags,
ID<Property> _property,
u8_t         _capacity)
: id(_index)
, node(_node)
, flags(_flags)
, property(_property)
, capacity(_capacity)
{
    if( capacity != 0 )
    {
        flags |= SlotFlag_IS_NOT_FULL;
    }
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
    return node == other.node && flags == other.flags && property == other.property && id == other.id;
}

bool Slot::operator!=(const Slot& other) const
{
    return node != other.node || flags != other.flags || property != other.property || id != other.id;
}

size_t Slot::adjacent_count() const
{
    return m_adjacent.size();
}

bool Slot::is_full() const
{
    return 0 == (flags & SlotFlag_IS_NOT_FULL);
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
    FW_EXPECT( _ref != *this, "Reflexive edge not handled" );
    FW_EXPECT( _ref.flags & flags & SlotFlag_TYPE_MASK, "Slot must have common type" );
    FW_EXPECT( !is_full(), "Slot is full" );
    m_adjacent.emplace_back(_ref);
    if ( m_adjacent.size() == capacity )
    {
        flags &= ~SlotFlag_IS_NOT_FULL; // Make sure IS_NOT_FULL is 0
    }
}

void Slot::remove_adjacent( const SlotRef& _ref )
{
    auto it = std::find( m_adjacent.begin(), m_adjacent.end(), _ref);
    FW_EXPECT( it != m_adjacent.end(), "SlotRef not found")
    m_adjacent.erase( it );
    flags |= SlotFlag_IS_NOT_FULL;
}

void Slot::allow( SlotFlags _flags)
{
    flags |= _flags;
}

SlotFlags Slot::type() const
{
    return flags & SlotFlag_TYPE_MASK;
}

bool Slot::empty() const
{
    return m_adjacent.empty();
}

const std::vector<SlotRef>& Slot::adjacent() const
{
    return m_adjacent;
}
