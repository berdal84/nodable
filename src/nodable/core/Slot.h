#pragma once
#include <vector>
#include "fw/core/types.h"
#include "Connector.h"
#include "Edge.h"

namespace ndbl
{
    // Forward declaration
    class Property;

    constexpr u8_t EDGE_PER_SLOT_MAX_COUNT = std::numeric_limits<u8_t>::max();

    class Slot
    {
    public:
        static const Slot null;

        u8_t              index;      // index in the parent SlotBag
        Connector         connector;  // The corresponding property connector (Input, Output, or both)
        u8_t              capacity;   // edge count max
        std::vector<Edge> edges;      // Edges connected to this slot

        Slot();
        Slot(const Slot& other);

        Connector first_adjacent_connector() const; // a slot can be connected to many others
        Connector adjacent_connector_at(u8_t pos) const;
        Property* get_property() const; // A slot wraps a Connector related to a given Property
        u8_t edge_count() const;
        bool is_full() const; // Slots have a capacity
        bool operator==(const Slot&) const;
        bool operator!=(const Slot&) const;
    };
}
