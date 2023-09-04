#include "Slot.h"
#include "Connector.h"


using namespace ndbl;

const Slot Slot::null{};

Slot::Slot()
: index(0)
, connector()
, capacity(0)
, edges()
{}

Slot::Slot(const Slot &other)
: index(other.index)
, connector(other.connector)
, capacity(other.capacity)
, edges(other.edges)
{}

Connector Slot::first_adjacent_connector() const
{
    if (!edges.empty())
    {
        auto first = *edges.cbegin();
        if (connector.way == Way::In)
        {
            return first.tail;
        }
        return first.head;
    }
    return {};
}
Connector Slot::adjacent_connector_at(u8_t pos) const
{
    FW_EXPECT(false, "TODO: implement to get the nth adjacent connector. Adapt first_adjacent_connector to be generalist");
}

Property* Slot::get_property() const
{
    return connector.get_property();
}

bool Slot::operator==(const Slot& other) const
{
    return connector == other.connector && index == other.index;
}

bool Slot::operator!=(const Slot& other) const
{
    return connector != other.connector || index != other.index;
}
u8_t Slot::edge_count() const
{ return edges.size(); }

bool Slot::is_full() const
{ return edges.size() < capacity; }
