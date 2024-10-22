#include "Slot.h"
#include "Node.h"
#include "Scope.h"

namespace ndbl
{
#define IS_COMPONENT_GUARD(Class) static_assert(std::is_base_of_v<NodeComponent, Class>, "Class is not a Component");
    class VariableNode;

    namespace Utils
    {
        template<class C>
        std::vector<C*> get_components(const std::vector<Node*>&);

        template<typename C>
        C* adjacent_component_at(const Node*, SlotFlags, u8_t pos);

        template<typename C>
        static std::vector<C*> adjacent_components(const Node*, SlotFlags);

        std::vector<Node*> get_adjacent_nodes(const Node*, SlotFlags);
        Node*              adjacent_node_at(const Node*, SlotFlags, u8_t pos);
        bool               is_instruction(const Node*);
        bool               can_be_instruction(const Node*);
        bool               is_unary_operator(const Node*);
        bool               is_binary_operator(const Node*);
        bool               is_conditional(const Node*);
    }

    template<class C>
    std::vector<C*> Utils::get_components(const std::vector<Node*>& nodes)
    {
        static_assert( IsNodeComponent<C>::value );

        std::vector<C*> result;
        result.reserve( nodes.size() );

        for(auto& node : nodes)
            result.push_back( node->get_component<C>() );

        return result;
    }

    template<typename ComponentT>
    ComponentT* Utils::adjacent_component_at(const Node* _node, SlotFlags _flags, u8_t _pos)
    {
        IS_COMPONENT_GUARD(ComponentT)
        if( Node* adjacent_node = adjacent_node_at(_node, _flags, _pos) )
        {
            return adjacent_node->get_component<ComponentT>();
        }
        return {};
    }

    template<typename ComponentT>
    static std::vector<ComponentT*> Utils::adjacent_components(const Node* _node, SlotFlags _flags)
    {
        IS_COMPONENT_GUARD(ComponentT)
        std::vector<ComponentT*> result;
        auto adjacent_nodes = get_adjacent_nodes( _node, _flags );
        for(auto adjacent_node : adjacent_nodes )
        {
            if( ComponentT* component = adjacent_node->get_component<ComponentT>() )
            {
                result.push_back( component );
            }
        }
        return result;
    }
}
