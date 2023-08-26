#pragma once

#include "fw/core/reflection/reflection" // for reflection MACROS
#include "fw/core/Pool.h"
#include "core/Property.h"

namespace ndbl
{
    // forward declarations
    class Node;
    using fw::pool::ID;

    /**
     * @enum Possible types for an edge
     */
    enum class Edge_t
    {
        IS_CHILD_OF,
        IS_INPUT_OF,
        IS_SUCCESSOR_OF,
        IS_PREDECESSOR_OF,
        IS_OUTPUT_OF
    };

    /** @static convert EdgeType to string */
    R_ENUM(Edge_t)
        R_ENUM_VALUE(IS_CHILD_OF)
        R_ENUM_VALUE(IS_INPUT_OF)
        R_ENUM_VALUE(IS_SUCCESSOR_OF)
        R_ENUM_VALUE(IS_PREDECESSOR_OF)
        R_ENUM_VALUE(IS_OUTPUT_OF)
    R_ENUM_END

    /**
     * @class Class to represent a property to property oriented edge and its nature.
     * Order is important.
     *
     * @example
     * @code
     * node_a.this IS_CHILD_OF       node_b.this
     * node_x.this IS_SUCCESSOR_OF   node_y.this
     * node_y.this IS_PREDECESSOR_OF node_x.this (express exactly the same)
     */
    class DirectedEdge
    {
    public:
        DirectedEdge() = delete;
        DirectedEdge(Property* _src, Edge_t _type, Property* _dst);        // Connect source to target with a given edge type.
        DirectedEdge(Property* _src, Property* _dst);                      // Connect source to target with the edge IS_INPUT_OF
        DirectedEdge(Edge_t, std::pair<Property*,Property*>);              // Connect a pair of property (source, target) with a given edge type.
        DirectedEdge(Node *_src, Edge_t _type, Node *_dst);                // Connect source["this"] to target["this"] with a given edge type.
        DirectedEdge(ID<Node> _src, Edge_t _type, ID<Node> _dst);  // Connect source["this"] to target["this"] with a given edge type.

        bool operator==(const DirectedEdge&) const;  // Compare (type, nodes, and direction) two edges.

        bool                            is_connected_to(const ID<Node> _node)const; // Check if a given node is connected to this edge.
        std::pair<Node*, Node*>         nodes() const; // Get the pair of nodes connected by this edge
        std::pair<Property*, Property*> props() const { return m_props; } // get the pair of properties connected by this edge
        Edge_t                          type() const { return m_type; } // Nature of the edge
        Property*                       src() const { return m_props.first; }
        Node*                           src_node() const { return src()->owner().get(); }
        Property*                       dst() const { return m_props.second; }
        Node*                           dst_node() const { return dst()->owner().get(); }
    protected:
        static void                     sanitize(DirectedEdge*);  // ensure a given edge is well-formed.

        std::pair<Property*, Property*> m_props; // <Source, Destination>
        Edge_t                          m_type;
    };

}