#pragma once
#include <vector>
#include "fw/core/types.h"
#include "Edge.h"
#include "Way.h"

namespace ndbl
{
    // Forward declaration
    class Node;
    class Property;
    using fw::pool::ID;

    constexpr u8_t EDGE_PER_SLOT_MAX_COUNT = std::numeric_limits<u8_t>::max();

    class Slot
    {
    public:
        static const Slot null;

        u8_t              index;      // slot index (in node's SlotBag)
        ID<Node>          node;       // node id (in fw::pool::Pool)
        Way               way;        // possible way a slot can be connected (in, out, both, none)
        u8_t              property;   // property index (in node's PropertyBag)
        u8_t              capacity;   // edge max count
        std::vector<Edge> edges;      // edges connected (heading or not this slot)

        Slot();
        Slot(const Slot& other);

        Node*     get_node() const;
        Slot      first_adjacent_slot() const; // a slot can be connected to many others
        Slot      adjacent_slot_at(u8_t pos) const;
        Property* get_property() const; // A slot wraps a Slot related to a given Property
        bool      allows(Way way) const;
        u8_t      edge_count() const;
        bool      is_full() const; // Slots have a capacity
        bool operator==(const Slot&) const;
        bool operator!=(const Slot&) const;
        operator bool () const;
    };
}
