#include "Slot.h"
#include "Node.h"
#include "Scope.h"

namespace ndbl
{
#define IS_COMPONENT_GUARD(Class) static_assert(std::is_base_of_v<Component, Class>, "Class is not a Component");
    class VariableNode;

    // Static functions to make life easier with graph related stuff
    class GraphUtil
    {
    public:
        template<typename ComponentT>
        static PoolID<ComponentT> adjacent_component_at(const Node* _node, SlotFlags _flags, u8_t _pos)
        {
            IS_COMPONENT_GUARD(ComponentT)
            if( Node* adjacent_node = adjacent_node_at(_node, _flags, _pos).get() )
            {
                return adjacent_node->get_component<ComponentT>();
            }
            return {};
        }

        template<typename ComponentT>
        static std::vector<PoolID<ComponentT>> adjacent_components(const Node* _node, SlotFlags _flags)
        {
            IS_COMPONENT_GUARD(ComponentT)
            std::vector<PoolID<ComponentT>> result;
            auto adjacent_nodes = get_adjacent_nodes( _node, _flags );
            for(auto adjacent_node : adjacent_nodes )
            {
                if( PoolID<ComponentT> component = adjacent_node->get_component<ComponentT>() )
                {
                    result.push_back( component );
                }
            }
            return result;
        }

        static  std::vector<PoolID<Node>> get_adjacent_nodes(const Node* _node, SlotFlags _flags)
        {
            std::vector<PoolID<Node>> result;
            for ( Slot* slot : _node->filter_slots( _flags ) )
            {
                for( const SlotRef& adjacent : slot->adjacent() )
                {
                    result.emplace_back( adjacent.node );
                }
            }
            return result;
        }

        static PoolID<Node> adjacent_node_at(const Node* _node, SlotFlags _flags, u8_t _pos)
        {
            if ( Slot* adjacent_slot = _node->slots.find_adjacent_at( _flags, _pos ) )
            {
                return adjacent_slot->node;
            }
            return {};
        }
    };
}
