#include "Slot.h"
#include "SlotRef.h"
#include "Node.h"

using namespace ndbl;

const Slot Slot::null{};

Slot::Slot()
: flags(SlotFlag::SlotFlag_NONE )
, capacity(0)
{}

Slot::Slot(const Slot &other)
: id(other.id )
, node(other.node)
, property(other.property)
, flags(other.flags)
, capacity(other.capacity)
, adjacent(other.adjacent )
{}


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
{}

SlotRef Slot::first_adjacent() const
{
    if (!adjacent.empty())
    {
        return adjacent[0];
    }
    return SlotRef::null;
}

SlotRef Slot::adjacent_at(u8_t pos) const
{
    u8_t count{0};
    for (auto& each : adjacent )
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
    return adjacent.size();
}

bool Slot::is_full() const
{
    return adjacent.size() >= capacity;
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
    adjacent.emplace_back(_ref);
}

void Slot::remove_adjacent( const SlotRef& _ref )
{
    auto it = std::find( adjacent.begin(), adjacent.end(), _ref);
    FW_EXPECT( it != adjacent.end(), "SlotRef not found")
    adjacent.erase( it );
}

void Slot::allow( SlotFlags _flags)
{
    flags |= _flags;
}

void Slot::set_capacity( u8_t _capacity )
{
    FW_EXPECT( adjacent.empty() || adjacent.size() < _capacity, "Cannot set a capacity below the edge count");
    capacity = _capacity;
}

SlotFlags Slot::type() const
{
    return flags & SlotFlag_TYPE_MASK;
}
