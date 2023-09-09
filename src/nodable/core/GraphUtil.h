#include "Slot.h"
#include "Node.h"
#include "Scope.h"

namespace ndbl
{
#define IS_COMPONENT_GUARD(Class) static_assert(std::is_base_of_v<Component, Class>, "Class is not a Component");

    // Static functions to make life easier with graph related stuff
    class GraphUtil
    {
    public:
        template<typename ComponentT>
        static PoolID<ComponentT> adjacent_component_at(const Node* _node, Relation _relation, Way _way, u8_t _pos)
        {
            IS_COMPONENT_GUARD(ComponentT)
            if( Node* adjacent_node = adjacent_node_at(_node, _relation, _way, _pos).get() )
            {
                return adjacent_node->get_component<ComponentT>();
            }
            return {};
        }

        template<typename ComponentT>
        static std::vector<PoolID<ComponentT>> adjacent_components(const Node* _node, Relation _relation, Way _way)
        {
            IS_COMPONENT_GUARD(ComponentT)
            std::vector<PoolID<ComponentT>> result;
            auto adjacent_nodes = get_adjacent_nodes( _node, _relation, _way );
            for(auto adjacent_node : adjacent_nodes )
            {
                if( PoolID<ComponentT> component = adjacent_node->get_component<ComponentT>() )
                {
                    result.push_back( component );
                }
            }
            return result;
        }

        static  std::vector<PoolID<Node>> get_adjacent_nodes(const Node* _node, Relation _relation, Way _way)
        {
            FW_EXPECT(false, "TODO: implement");
        }

        static PoolID<Node> adjacent_node_at(const Node* _node, Relation _relation, Way _way, u8_t _pos)
        {
            DirectedEdge edge = _node->slots.find_edge_at(_relation, _way, _pos);
            if (edge == DirectedEdge::null )
            {
                return {};
            }
            Slot slot = _way == Way::Out ? edge.tail : edge.head;
            return slot.node;
        }
    };
}
