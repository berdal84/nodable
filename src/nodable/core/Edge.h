#pragma once
#include "TDirectedEdge.h"
#include "Relation.h"

namespace ndbl
{
    // Forward declarations
    class Slot;
    using Edge = TDirectedEdge<Slot, Relation>;

    static Edge& sanitize_edge(Edge& _edge);
} // namespace ndbl