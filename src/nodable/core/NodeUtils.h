#pragma once
#include <vector>
#include <unordered_set>
#include "fw/core/Pool.h"
#include "Node.h"

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
        get_components(const std::vector<ID<Node>>&);

        template<class ComponentT>
        static std::vector<ID<ComponentT>>
        get_component_ids(const std::vector<ID<Node>>&);

    private:
        static NodeUtils s_instance;
    };

    template<class ComponentT>
    std::vector<ID<ComponentT>> NodeUtils::get_component_ids(const std::vector<ID<Node>>& nodes)
    {
        if ( nodes.empty() )
        {
            return {};
        }

        std::vector<ID<ComponentT>> result;
        result.reserve( nodes.size() );

        auto get_component = [](PoolID<Node> node ) { return node->get_component<ComponentT>(); };
        std::transform( nodes.begin(), nodes.end(), result.end(), get_component );

        return result; // wil be moved
    }

    template<class ComponentT>
    std::vector<ComponentT*> NodeUtils::get_components(const std::vector<ID<Node>>& nodes)
    {
        // Get ComponentT on each node
        auto components_ids =  NodeUtils::get_component_ids<ComponentT>(nodes);

        if( components_ids.empty() )
        {
            return {};
        }

        std::vector<ComponentT*> result;
        result.reserve(components_ids.size());

        auto get_pointer = [](ID<ComponentT> component_id ) { return component_id.get(); };
        std::transform( components_ids.begin(), components_ids.end(), result.end(), get_pointer);

        auto is_null = [](ComponentT* component ) { return component == nullptr; };
        std::remove_if( result.begin(), result.end(), is_null );

        return result; // wil be moved
    }
}
