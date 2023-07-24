#include "ComponentManager.h"
#include "Component.h"
#include "Node.h"

using namespace ndbl;

std::unordered_set<Component*> ComponentManager::s_components;

void ComponentManager::destroy(Component* component)
{
    FW_EXPECT(component != nullptr, "Can't destroy a nullptr component")
    s_components.erase(component);
    delete component;
}

void ComponentManager::attach(Component* component, Node* node)
{
    FW_EXPECT(node != nullptr, "Can't attach a component on a nullptr Node*")
    node->components.add(component);
}

void ComponentManager::detach(Component* component, Node* node)
{
    FW_EXPECT(node != nullptr, "Can't remove a component from a nullptr Node*")
    node->components.remove(component);
}

ComponentManager::Stats ComponentManager::get_stats()
{
    Stats stats;
    stats.component_count = s_components.size();
    return stats;
}