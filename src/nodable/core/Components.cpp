#include "Components.h"
#include "Component.h"

using namespace ndbl;

void
Components::add(Component *component)
{
    m_components_by_type.emplace(component->get_type()->index(), component);
    m_components.push_back(component);
    component->set_owner(m_owner);
}

void
Components::remove(Component *component)
{
    auto found = std::find(m_components.begin(), m_components.end(), component);
    FW_EXPECT(found != m_components.end(), "Component can't be found it those components");
    m_components_by_type.erase(component->get_type()->index());
    m_components.erase(found);
    component->set_owner(nullptr);
}

Component* Components::get(const fw::type* desired_type) const
{
    // Search with class name
    {
        auto it = m_components_by_type.find(desired_type->index() );
        if (it != m_components_by_type.end())
        {
            return it->second;
        }
    }

    // Search for a derived class
    for (const auto & [name, component] : m_components_by_type)
    {
        if ( component->get_type()->is_child_of(desired_type) )
        {
            return component;
        }
    }

    return nullptr;
}

std::vector<Component*> Components::get_all()
{
    return m_components;
}
