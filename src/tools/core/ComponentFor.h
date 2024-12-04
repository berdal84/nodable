#pragma once
#include <string>
#include "reflection/reflection"
#include "Signals.h"

namespace tools
{
    // Basic component holding no data (void)
    template<class EntityT>
    class ComponentFor
    {
    public:
        DECLARE_REFLECT_virtual

        SIGNAL(on_owner_init , EntityT*           );
        SIGNAL(on_name_change, const std::string& );

        ComponentFor();
        virtual ~ComponentFor() = default;

        void                set_name(std::string name);
        const std::string&  name() const  { return m_name; }
        EntityT*            entity() const { return m_owner; }

        void set_owner(EntityT* entity)
        {
            VERIFY( entity != nullptr, "entity is null");
            VERIFY( m_owner == nullptr, "set_owner(..) must be called once!");

            m_owner = entity;
            on_owner_init.emit(entity);
        }

        EntityT*    m_owner = nullptr;
        std::string m_name;
    };

    template<class EntityT>
    ComponentFor<EntityT>::ComponentFor()
        : m_owner(nullptr) // is set by Entity
        , m_name( get_class()->name() )
    {
    }

    template<class EntityT>
    void ComponentFor<EntityT>::set_name(std::string name)
    {
        m_name = std::move(name);
        on_name_change.emit(m_name );
    }
}
