#include "Slot.h"
#include "ASTNode.h"
#include "ASTScope.h"

namespace ndbl
{
#define IS_COMPONENT_GUARD(Class) static_assert(std::is_base_of_v<ASTNodeComponent, Class>, "Class is not a Component");
    class ASTVariableNode;

    /**
     * Static class responsible for create_new/destroy/attach/remove components
     * All Components are owned by the manager.
     * Therefore new / delete are handled by this class.
     */
    namespace ASTUtils
    {
        template<class C>             std::vector<C*>          get_components(const std::vector<ASTNode*>&);
        template<typename ComponentT> ComponentT*              adjacent_component_at(const ASTNode* _node, SlotFlags _flags, u8_t _pos);
        template<typename ComponentT> std::vector<ComponentT*> adjacent_components(const ASTNode* _node, SlotFlags _flags);

        std::vector<ASTNode*> get_adjacent_nodes(const ASTNode* _node, SlotFlags _flags);
        ASTNode*              adjacent_node_at(const ASTNode* _node, SlotFlags _flags, u8_t _pos);
    };

    template<class C>
    std::vector<C*> ASTUtils::get_components(const std::vector<ASTNode*>& nodes)
    {
        static_assert(IsASTNodeComponent<C>::value );

        std::vector<C*> result;
        result.reserve( nodes.size() );

        for(auto& node : nodes)
            result.push_back( node->get_component<C>() );

        return result;
    }

    // Static functions to make life easier with graph related stuff
    template<typename ComponentT>
    ComponentT* ASTUtils::adjacent_component_at(const ASTNode* _node, SlotFlags _flags, u8_t _pos)
    {
        IS_COMPONENT_GUARD(ComponentT)
        if( ASTNode* adjacent_node = adjacent_node_at(_node, _flags, _pos) )
        {
            return adjacent_node->get_component<ComponentT>();
        }
        return {};
    }

    template<typename ComponentT>
    std::vector<ComponentT*> ASTUtils::adjacent_components(const ASTNode* _node, SlotFlags _flags)
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
