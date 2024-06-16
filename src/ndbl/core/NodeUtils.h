#pragma once
#include "Node.h"
#include "tools/core/memory/memory.h"
#include <unordered_set>
#include <vector>

namespace ndbl
{
    // forward declarations
    class NodeComponent;

    /**
     * Static class responsible for create/destroy/attach/remove components
     * All Components are owned by the manager.
     * Therefore new / delete are handled by this class.
     */
    namespace NodeUtils
    {
        template<class C>
        static std::vector<C*> get_components(const std::vector<Node*>&);
    };

    template<class C>
    std::vector<C*> NodeUtils::get_components(const std::vector<Node*>& nodes)
    {
        static_assert( IsNodeComponent<C>::value );

        std::vector<C*> result;
        result.reserve( nodes.size() );

        for(auto& node : nodes)
            result.push_back( node->get_component<C>() );

        return result;
    }
}
