#include "Slot.h"
#include "Node.h"

using namespace ndbl;

const Slot Slot::null{};

Slot::Slot()
: index(0)
, node(0)
, property(0)
, way(Way::None)
, capacity(0)
, edges()
{}

Slot::Slot(const Slot &other)
: index(other.index)
, node(other.node)
, property(other.property)
, way(other.way)
, capacity(other.capacity)
, edges(other.edges)
{}


Slot::Slot(
u8_t     _index,
ID<Node> _node,
Way      _way,
u8_t     _property,
u8_t     _capacity,
std::vector<DirectedEdge>&& _edges)
: index(_index)
, node(_node)
, way(_way)
, property(_property)
, capacity(_capacity)
, edges(std::move(_edges))
{}

Slot Slot::first_adjacent_slot() const
{
    if (!edges.empty())
    {
        auto first = *edges.cbegin();
        if (way == Way::In)
        {
            return first.tail;
        }
        return first.head;
    }
    return {};
}

Slot Slot::adjacent_slot_at(u8_t pos) const
{
    u8_t count{0};
    for (const auto& edge : edges)
    {
        if( count == pos )
        {
            return edge.tail == *this ? edge.head : edge.tail;
        }
        ++count;
    }
    return {};
}

bool Slot::operator==(const Slot& other) const
{
    return node == other.node && way == other.way && property == other.property && index == other.index;
}

bool Slot::operator!=(const Slot& other) const
{
    return node != other.node || way != other.way || property != other.property || index != other.index;
}

u8_t Slot::edge_count() const
{
    return edges.size();
}

bool Slot::is_full() const
{
    return edges.size() >= capacity;
}

Slot::operator bool() const
{
    return *this != null;
}

Property* Slot::get_property() const
{
    return node->get_prop_at( property );
}

Node* Slot::get_node() const
{
    return node.get();
}


bool Slot::allows(Way desired_way) const
{
    return static_cast<u8_t>(way) & static_cast<u8_t>(desired_way);
}

bool Slot::allows(Relation _relation) const
{
    return !allowed_relation.empty() || (allowed_relation.find(_relation) == allowed_relation.end());
}

void Slot::add_edge(DirectedEdge _edge)
{
    FW_EXPECT( allows( _edge.relation ), "Relation not allowed" );
    FW_EXPECT( !is_full(), "Slot is full" );
    edges.emplace_back(std::move(_edge));
}

