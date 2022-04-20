#pragma once

#include <nodable/core/reflection/reflection> // for reflection MACROS

namespace Nodable
{
    class Node; // forward declare

    enum class EdgeType
    {
        IS_CHILD_OF,
        IS_INPUT_OF,
        IS_SUCCESSOR_OF,
        IS_PREDECESSOR_OF,
        IS_OUTPUT_OF
    };

    R_ENUM(EdgeType)
        R_ENUM_VALUE(IS_CHILD_OF)
        R_ENUM_VALUE(IS_INPUT_OF)
        R_ENUM_VALUE(IS_SUCCESSOR_OF)
        R_ENUM_VALUE(IS_PREDECESSOR_OF)
        R_ENUM_VALUE(IS_OUTPUT_OF)
    R_ENUM_END

    template<typename T>
    struct Pair
    {
        Pair() = delete;
        Pair(T _src, T _dst): src(_src), dst(_dst){}

        T src;
        T dst;

        friend bool operator==(const Pair<T>& _left, const Pair<T>& _right)
        {
            return (_left.src == _right.src)
                   && (_left.dst == _right.dst);
        }

        void swap()
        {
            std::swap(src, dst);
        }
    };

    /**
     * @brief A directed edge is a structure to express the nature of a link between two nodes in a specific direction.
     * It means you can't swap the nodes while loosing the link information.
     * ex: node_a IS_CHILD_OF       node_b
     *     node_x IS_SUCCESSOR_OF   node_y
     *     node_y IS_PREDECESSOR_OF node_x (express exactly the same)
     */
    class DirectedEdge
    {
    public:

        DirectedEdge() = delete;
        DirectedEdge(EdgeType _type, Node* _src, Node* _dst): type(_type), nodes(_src, _dst){ sanitize(); }
        DirectedEdge(Node* _src, EdgeType _type, Node* _dst): type(_type), nodes(_src, _dst){ sanitize(); }
        DirectedEdge(EdgeType _type, const Pair<Node*> _nodes) : type(_type), nodes(_nodes){ sanitize(); }

        EdgeType    type;
        Pair<Node*> nodes;

        bool is_about(Node* _node)const { return nodes.src == _node || nodes.dst == _node; }

        friend bool operator==(const DirectedEdge& _left_relation, const DirectedEdge& _right_relation)
        {
            return (_left_relation.type == _right_relation.type)
                && (_left_relation.nodes == _right_relation.nodes);
        }

    protected:
        virtual void sanitize()
        {
            if (type == EdgeType::IS_PREDECESSOR_OF ) // we never want this type of relation in our database
            {
                type = EdgeType::IS_SUCCESSOR_OF;
                nodes.swap();
            }
        }
    };

}