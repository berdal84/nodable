#include "Component.h"
#include "Node.h"

using namespace ndbl;

REGISTER
{
    fw::registration::push_class<Component>("Component");
}

Component::Component()
{}

void Component::set_owner(ID<Node> node)
{ m_owner = node; }
