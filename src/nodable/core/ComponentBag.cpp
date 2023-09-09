#include "ComponentBag.h"
#include "Component.h"

using namespace ndbl;
using namespace fw::pool;

void
ComponentBag::add(PoolID<Component> id)
{
    Component* component = id.get();
    const fw::type* type = component->get_type();
    std::type_index type_id = type->id();

    m_components_by_type.emplace( type_id, id );
    m_components.push_back( id );
    component->set_owner( m_owner );
}

void
ComponentBag::remove(PoolID<Component> component)
{
    auto found = std::find(m_components.begin(), m_components.end(), component );
    FW_EXPECT(found != m_components.end(), "Component can't be found it those components");
    m_components_by_type.erase(component->get_type()->id());
    m_components.erase(found);
    component->set_owner({});
}

void ComponentBag::set_owner(PoolID<Node> owner)
{
    m_owner = owner;
    for(auto each_component : m_components )
    {
        each_component->set_owner( owner );
    }
}
