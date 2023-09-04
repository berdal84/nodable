#pragma once
#include "fw/core/Pool.h"
#include "Way.h"

namespace ndbl
{
    // Forward declarations
    class Node;
    class Property;
    using fw::pool::ID;

    class Connector
    {
    public:
        static const Connector null;

        ID<Node>   node;
        Way        way;
        u8_t       property;

        Property*  get_property() const;
        Node*      get_node() const;

        Connector();
        Connector(const Connector&);
        operator bool () const;
        Connector& operator=(const Connector& other);
        bool operator==(const Connector& other) const;
        bool operator!=(const Connector& other) const;
        bool allows(Way way) const;
    };
} // namespace ndbl