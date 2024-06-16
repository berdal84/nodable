#pragma once

#include "tools/core/reflection/reflection"

namespace ndbl
{
    class ComponentFactory
    {
    public:
#if !TOOLS_POOL_ENABLE
        template<typename Component, typename ...Args>
        Component* create(Args... args)
        {
            auto instance = new Component(args...);
            stats.create_count++;
            return instance;
        }
#else
        template<typename Component, typename ...Args>
        tools::Component> create(Args... args)
        {
            auto *pool = tools::get_pool_manager()->get_pool();
            auto instance = pool->create<Component>(args...);
            stats.create_count++;
            return instance;
        }
#endif
    private:
        struct Stats
        {
            size_t create_count{0};
        } stats{};
    };

    ComponentFactory* get_component_factory();
    ComponentFactory* init_component_factory();
    void shutdown_component_factory();
}