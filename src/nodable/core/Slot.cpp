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
    FW_EXPECT(false, "TODO: implement to get the nth adjacent slot. Adapt first_adjacent_slot to be generalist");
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
    return edges.size() < capacity;
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

