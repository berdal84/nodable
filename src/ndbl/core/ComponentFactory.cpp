#include "ComponentFactory.h"

using namespace ndbl;
using namespace tools;

static ComponentFactory* g_component_factory{nullptr};

ComponentFactory* ndbl::get_component_factory()
{
    ASSERT(g_component_factory != nullptr);
    return g_component_factory;
}

ComponentFactory* ndbl::init_component_factory()
{
    ASSERT(g_component_factory == nullptr)
    g_component_factory = new ComponentFactory();
    return g_component_factory;
}

void ndbl::shutdown_component_factory()
{
    delete g_component_factory;
    g_component_factory = nullptr;
}