#include "Component.h"
#include "Node.h"

using namespace ndbl;
using namespace fw;

REGISTER
{
    registration::push_class<Component>("Component");
}

Component::Component()
{}

void Component::set_owner(PoolID<Node> node)
{ m_owner = node; }
