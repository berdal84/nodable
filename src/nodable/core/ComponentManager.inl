#pragma once
#include "ComponentManager.h"

namespace ndbl
{

template<class ComponentT, typename ...Args>
ComponentT* ComponentManager::create(Args... args)
{
    ComponentT* component = new ComponentT(args...);
    s_components.insert(component);
    return component;
}

template<class T>
std::vector<T*> ComponentManager::collect(const std::vector<Node*>& nodes)
{
    std::vector<T*> components_found;

    auto get_component_if_exists = [&components_found](const Node* node) -> void
    {
        if (T* view = node->components.get<T>())
            components_found.push_back(view);
    };

    components_found.reserve(nodes.size()); // we anticipate the worst case scenario when each node has the expected component
    std::for_each(nodes.begin(), nodes.end(), get_component_if_exists);

    return components_found; // wil be moved
}

template<class... Types>
void ComponentManager::init_for()
{
    auto types = fw::type::get_all<Types...>::types();
    LOG_MESSAGE("ComponentManager", "Initializing for ...\n");

    for(const fw::type* each_type : types )
    {
        LOG_MESSAGE("ComponentManager", "- %s\n", each_type->get_name() );

        // TODO: instantiate N pools (one per Type)
        //       each pool will be responsible for storing a given Type contiguously in memory.
    }

    LOG_MESSAGE("ComponentManager", "Initializing DONE.\n");
}

} // namespace ndbl