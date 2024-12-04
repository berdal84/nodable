#pragma once
#include <string>
#include "reflection/reflection"
#include "Signals.h"

namespace tools
{
    // forward declared to avoid a dependency with Node.h
    class Entity;

    class Component
    {
        friend class Entity;
    public:
        DECLARE_REFLECT_virtual

        SIGNAL(on_owner_init, Entity*            );
        SIGNAL(on_name_change  , const std::string& );

        Component();
        virtual ~Component() = default;

        void                set_name(std::string name);
        const std::string&  name() const  { return m_name; }
        Entity*             entity() const { ASSERT(m_owner); return m_owner; }

    private:
        void init_owner(Entity* entity)
        {
            m_owner = entity;
            on_owner_init.emit(entity);
        }

        Entity*     m_owner;
        std::string m_name;
    };
}