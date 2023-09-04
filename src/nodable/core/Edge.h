#pragma once
#include "Connector.h"
#include "DirectedEdge.h"
#include "Relation.h"

namespace ndbl
{
    using Edge = DirectedEdge<Connector, Relation>;

    Edge& sanitize_edge(Edge& _edge) // In case relation is not primary, swaps vertex and use complement relation, return the same edge otherwise.
    {
        if ( is_primary(_edge.relation) )
        {
            return _edge;
        }
        std::swap(_edge.tail, _edge.head);
        _edge.relation = complement(_edge.relation);
        return _edge;
    }
} // namespace ndbl