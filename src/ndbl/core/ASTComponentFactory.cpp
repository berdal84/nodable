#include "ASTComponentFactory.h"

using namespace ndbl;

ASTComponentFactory* g_component_factory = nullptr;

void ASTComponentFactory::destroy(ndbl::ASTNodeComponent* component)
{
#if !TOOLS_POOL_ENABLE
    delete component;
#else
    static_assert(false, "Not implemented for pool yet");
#endif
}

ASTComponentFactory* ndbl::get_component_factory()
{
    VERIFY(g_component_factory != nullptr, "Did you call init_component_factory first?");
    return g_component_factory;
}

ASTComponentFactory* ndbl::init_component_factory()
{
    ASSERT(g_component_factory == nullptr)
    g_component_factory = new ASTComponentFactory();
    return g_component_factory;
}

void ndbl::shutdown_component_factory(ASTComponentFactory* factory)
{
    ASSERT(g_component_factory == factory) // singleton
    delete g_component_factory;
    g_component_factory = nullptr;
}