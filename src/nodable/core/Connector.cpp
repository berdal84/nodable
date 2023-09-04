#include "Connector.h"
#include "Node.h"

using namespace ndbl;

const Connector Connector::null{};

Connector::Connector()
: node(0)
, property(0)
, way(Way::None)
{
}

Connector::Connector(const Connector& other)
: node( other.node )
, property( other.property )
, way( other.way )
{
}

bool Connector::operator==(const Connector &other) const
{
    return node == other.node && way == other.way && property == other.property;
}

bool Connector::operator!=(const Connector &other) const
{
    return node != other.node || way != other.way || property != other.property;
}

Property* Connector::get_property() const
{
    return node->get_prop_at( property );
}

Node* Connector::get_node() const
{
    return node.get();
}

Connector::operator bool() const
{
    return !(node.id == 0 && property == 0);
}

Connector& Connector::operator=(const Connector &other)
{
    node = other.node;
    property = other.property;
    way = other.way;
    return *this;
}

bool Connector::allows(Way desired_way) const
{
    return static_cast<u8_t>(way) & static_cast<u8_t>(desired_way);
}
