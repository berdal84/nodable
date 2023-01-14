#pragma once

#include <nodable/core/reflection/reflection> // for reflection MACROS

namespace ndbl
{
    class Node; // forward declare

    /**
     * @enum Possible types for an edge
     */
    enum class EdgeType
    {
        IS_CHILD_OF,
        IS_INPUT_OF,
        IS_SUCCESSOR_OF,
        IS_PREDECESSOR_OF,
        IS_OUTPUT_OF
    };

    /** @static convert EdgeType to string */
    R_ENUM(EdgeType)
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
     * @class Class to represent a Node to Node oriented relation and its nature.
     * Order is important.
     *
     * @example
     * @code
     * node_a IS_CHILD_OF       node_b
     * node_x IS_SUCCESSOR_OF   node_y
     * node_y IS_PREDECESSOR_OF node_x (express exactly the same)
     */
    class DirectedEdge
    {
    public:
        Pair<Node*> nodes; // Node pair (source and destination)
        EdgeType    type;  // Nature of the relation

        DirectedEdge() = delete;
        DirectedEdge(EdgeType _type, Node* _src, Node* _dst): type(_type), nodes(_src, _dst){ sanitize(); }
        DirectedEdge(Node* _src, EdgeType _type, Node* _dst): type(_type), nodes(_src, _dst){ sanitize(); }
        DirectedEdge(EdgeType _type, const Pair<Node*> _nodes) : type(_type), nodes(_nodes){ sanitize(); }

        bool is_about(Node* _node)const { return nodes.src == _node || nodes.dst == _node; }  // Check it a given node is connected to this edge
        friend bool operator==(const DirectedEdge& left_edge, const DirectedEdge& right_edge) // Compare (type, nodes, and direction) two edges
        {
            return (left_edge.type == right_edge.type)
                && (left_edge.nodes == right_edge.nodes);
        }

    protected:
        void sanitize() // ensure the edge is well-formed
        {
            if (type == EdgeType::IS_PREDECESSOR_OF ) // we never want this type of relation in our database
            {
                type = EdgeType::IS_SUCCESSOR_OF;
                nodes.swap();
            }
        }
    };

}