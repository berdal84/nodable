#pragma once
#include <vector>
#include <unordered_set>
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
    class ComponentManager {
    public:
        struct Stats
        {
            size_t component_count  = 0;
        };
        template<class ...Args>
        static void init_for();
        template<class ComponentT, typename ...Args>
        static ComponentT*   create(Args...);
        static void          destroy(Component*);
        static void          attach(Component*, Node*);
        static void          detach(Component*, Node*);
        static Stats         get_stats();
        template<class ComponentT>
        static std::vector<ComponentT*> collect(const std::vector<Node*> &nodes);
    private:
        static ComponentManager s_instance;

        // TODO: Use a different container to ensure data is contiguous.
        //       The problem with std::vector<T*> is pointers are contiguous, but pointed data not.
        //       We need a container holding Ts, able to be resized while being able to retrieve data when the buffer
        //       changes location in memory. Using uuid seems to be a good option.
        //       size_t uuid = ... // existing uuid
        //       Component* my_component = ComponentManager::get(uuid);
        //
        //       s_components should be like an unordered_map (uuid => Component pointer)
        //       s_components_xxx should like be unordered_map (uuid => Specialized Component object )
        //
        static std::unordered_set<Component*>              s_components;
        static std::unordered_multimap<size_t, Component*> s_components_by_type;
    };
}

#include "ComponentManager.inl"
