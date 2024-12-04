#pragma once
#include <string>
#include "reflection/reflection"
#include "Signals.h"

namespace tools
{
    //
    // Class to extend to create a component for the entity of class E
    // see ComponentsOf<class E>
    //
    template<typename E>
    class ComponentFor
    {
    public:
        DECLARE_REFLECT_virtual

        SIGNAL(on_set_entity , E*           );
        SIGNAL(on_name_change, const std::string& );

        ComponentFor() = delete;
        ComponentFor(const char* name);
        virtual ~ComponentFor() = default;

        void                set_name(std::string name);
        const std::string&  name() const  { return m_name; }
        E*                  entity() const { return m_entity; }

        void set_entity(E* entity)
        {
            VERIFY( entity != nullptr, "entity is null");
            VERIFY(m_entity == nullptr, "set_entity(..) must be called once!");

            m_entity = entity;
            on_set_entity.emit(entity);
        }

        E* m_entity = nullptr;
        std::string m_name;
    };

    template<class E>
    ComponentFor<E>::ComponentFor(const char* name)
        : m_entity(nullptr) // is set by Entity
        , m_name(name)
    {
    }

    template<class E>
    void ComponentFor<E>::set_name(std::string name)
    {
        m_name = std::move(name);
        on_name_change.emit(m_name );
    }
}
