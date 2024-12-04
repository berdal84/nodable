#include "Component.h"

using namespace tools;

REFLECT_STATIC_INITIALIZER
(
    DEFINE_REFLECT(Component);
)

// do not move all those member declarations to .h, this code required to trigger static init above

Component::Component()
    : m_owner(nullptr) // is set by Entity
    , m_name( get_class()->name() )
{
}

void Component::set_name(std::string name)
{
    m_name = std::move(name);
    on_name_change.emit(m_name );
}
