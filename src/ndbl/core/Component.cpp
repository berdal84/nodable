#include "Component.h"
#include "Node.h"

using namespace ndbl;
using namespace tools;

REFLECT_STATIC_INIT
{
    StaticInitializer<Component>("Component");
}

Component::Component()
{}

void Component::set_owner(PoolID<Node> node)
{ m_owner = node; }
