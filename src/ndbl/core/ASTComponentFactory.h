#pragma once

#include "tools/core/reflection/reflection"
#include "ASTNodeComponent.h"

namespace ndbl
{
    class ASTComponentFactory
    {
    public:

#if !TOOLS_POOL_ENABLE

        void destroy(ndbl::ASTNodeComponent* component);

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


    [[nodiscard]]
    ASTComponentFactory* init_component_factory(); // note: store the ptr, you need it to shut it down
    ASTComponentFactory* get_component_factory();
    void                 shutdown_component_factory(ASTComponentFactory* factory); // undo init()
}