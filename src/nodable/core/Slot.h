#pragma once
#include <vector>
#include <set>
#include "fw/core/types.h"
#include "DirectedEdge.h"
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
        std::set<Relation> allowed_relation;
        std::vector<DirectedEdge> edges;      // edges connected (heading or not this slot)

        Slot();
        Slot(const Slot& other);
        Slot(u8_t _index, ID<Node> _node, Way _way = Way::Default, u8_t _property = 0, u8_t capacity = 0, std::vector<DirectedEdge>&& _edges = {});

        Node*     get_node() const;
        Slot      first_adjacent_slot() const; // a slot can be connected to many others
        Slot      adjacent_slot_at(u8_t pos) const;
        Property* get_property() const; // A slot wraps a Slot related to a given Property
        bool      allows(Way way) const;
        u8_t      edge_count() const;
        bool      is_full() const; // Slots have a capacity
        void      add_edge(DirectedEdge);
        bool      allows(Relation) const;
        bool operator==(const Slot&) const;
        bool operator!=(const Slot&) const;
        operator bool () const;
    };
}
