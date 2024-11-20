#include "NodeComponent.h"
#include "Node.h"

using namespace ndbl;
using namespace tools;

REFLECT_STATIC_INITIALIZER
(
    DEFINE_REFLECT(NodeComponent);
)

// do not move all those member declarations to .h, this code required to trigger static init above

NodeComponent::NodeComponent()
        : m_owner(nullptr)
        , m_name( get_class()->name() )
{
}

void NodeComponent::reset_owner(Node *owner)
{
    m_owner = owner;
    on_reset_owner.emit();
}

void NodeComponent::reset_name(std::string name)
{
    m_name = std::move(name);
}
