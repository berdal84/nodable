#pragma once

#include <nodable/core/reflection/reflection> // for reflection MACROS
#include <nodable/core/Property.h>

namespace ndbl
{
    // forward declarations
    class Node;

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
     * @struct Simple structure to store a pair of T, provide comparison and swap.
     * @tparam T the inner type for pair elements
     */
    template<typename T>
    struct Pair
    {
        Pair() = delete;
        Pair(T _src, T _dst): src(_src), dst(_dst){}

        T src;
        T dst;

        /** Compare two pairs (ex: (a,b) == (a,b), but (b,a) != (a,b) ) */
        friend bool operator==(const Pair<T>& _left, const Pair<T>& _right)
        {
            return (_left.src == _right.src)
                   && (_left.dst == _right.dst);
        }

        /** Swap the pair */
        void swap()
        {
            std::swap(src, dst);
        }
    };

    /**
     * @class Class to represent a property to property oriented relation and its nature.
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
        Pair<Property*> prop; // Source and destination
        Edge_t type;          // Nature of the relation

        DirectedEdge() = delete;
        DirectedEdge(Edge_t _type, Node* _src, Node * _dst);
        DirectedEdge(Edge_t _type, Property * _src, Property * _dst);
        DirectedEdge(Property * _src, Property * _dst);
        DirectedEdge(Property * _src, Edge_t _type, Property * _dst);
        DirectedEdge(Node * _src, Edge_t _type, Node * _dst);
        DirectedEdge(Edge_t _type, const Pair<Property*> _pair);

        bool is_about(Node* _node)const;            // Check if a given node is connected to this edge.
        bool operator==(const DirectedEdge&) const; // Compare (type, nodes, and direction) two edges.
    protected:
        void sanitize();                            // ensure the edge is well-formed.
    };

}