#pragma once
#include "Node.h"
#include "fw/core/memory/Pool.h"
#include <unordered_set>
#include <vector>

namespace ndbl
{
    // forward declarations
    class Component;

    /**
     * Static class responsible for create/destroy/attach/remove components
     * All Components are owned by the manager.
     * Therefore new / delete are handled by this class.
     */
    class NodeUtils {
    public:
        template<class ComponentT>
        static std::vector<ComponentT*>
        get_components(const std::vector<PoolID<Node>>&);

        template<class ComponentT>
        static std::vector<PoolID<ComponentT>>
        get_component_ids(const std::vector<PoolID<Node>>&);

    private:
        static NodeUtils s_instance;
    };

    template<class ComponentT>
    std::vector<PoolID<ComponentT>> NodeUtils::get_component_ids(const std::vector<PoolID<Node>>& nodes)
    {
        if ( nodes.empty() )
        {
            return {};
        }

        std::vector<PoolID<ComponentT>> result;
        result.reserve( nodes.size() );

        for(auto& node : nodes)
        {
            result.push_back( node->get_component<ComponentT>() );
        }

        return result; // wil be moved
    }

    template<class ComponentT>
    std::vector<ComponentT*> NodeUtils::get_components(const std::vector<PoolID<Node>>& nodes)
    {
        // Get ComponentT on each node
        auto components_ids =  NodeUtils::get_component_ids<ComponentT>(nodes);

        if( components_ids.empty() )
        {
            return {};
        }

        std::vector<ComponentT*> result;
        result.reserve(components_ids.size());

        for(auto& id : components_ids)
        {
            if(ComponentT* component = id.get() )
            {
                result.push_back(component);
            }
        }

        return result; // wil be moved
    }
}
