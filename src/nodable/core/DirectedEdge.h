#pragma once
#include "TDirectedEdge.h"
#include "Relation.h"

namespace ndbl
{
    // Forward declarations
    class Slot;
    using DirectedEdge = TDirectedEdge<Slot, Relation>;

    extern DirectedEdge& sanitize_edge(DirectedEdge& _edge);
} // namespace ndbl