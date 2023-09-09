#pragma once
#include "TDirectedEdge.h"
#include "Relation.h"

namespace ndbl
{
    // Forward declarations
    class Slot;
    typedef TDirectedEdge<Slot, Relation> DirectedEdge;

    static DirectedEdge& sanitize_edge(DirectedEdge& _edge);
} // namespace ndbl