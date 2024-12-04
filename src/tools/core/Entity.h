#pragma once

#include <string>
#include <memory>
#include <algorithm>
#include <unordered_map>

#include "Signals.h"
#include "types.h"
#include "assertions.h"
#include "memory/memory.h"
#include "reflection/reflection"
#include "Component.h"

namespace tools
{
    class Entity
	{
    public:
        DECLARE_REFLECT_virtual
        POOL_REGISTRABLE(Entity)

        typedef std::vector<Component*>::iterator       iterator;
        typedef std::vector<Component*>::const_iterator const_iterator;

        Entity() = default;
        ~Entity() { on_destroy.emit(); }

        SIGNAL(on_destroy);
        SIGNAL(on_name_change, const char *);

        void                 init(const std::string& name) { m_name = name; on_name_change.emit(m_name.c_str()); }
        const std::string&   name() const { return m_name; };

        template<typename T> bool
        has()const
        { return _find_by_type<T>() != m_components.end(); }

        const std::vector<Component*>& components(){ return m_components; }

        template<typename C, typename ...Args>
        C* emplace(Args...args)
        {
            C* component = new C(args...);
            _append(component);
            return component;
        }

        template<typename C>
        void remove(C* component)
        {
            auto found1 = std::find(m_components.begin(), m_components.end(), component );
            VERIFY(found1 != m_components.end(), "Component can't be found it those components");
            m_components.erase(found1);

            delete component;
        }

        template<typename C>
        C* get() const
        {
            auto it = _find_by_type<C>();
            if (it != m_components.end())
                return reinterpret_cast<C*>( const_cast<Component*>( *it ) );
            return nullptr;
        }

        template<class C>
        static std::vector<C*> get_all(const std::vector<Entity*>& entities)
        {
            std::vector<C*> result;
            result.reserve( entities.size() );

            for(Entity* _entity : entities)
                result.push_back(_entity->get<C>() );

            return result;
        }

        template<class C>
        C* require(const char* reason) const
        {
            C* component = get<C>();
            VERIFY(component != nullptr, reason);
            return component;
        }

    private:

        template<typename C>
        const_iterator _find_by_type() const
        {
            return std::find_if(
                m_components.cbegin(),
                m_components.cend(),
                [](Component* c )
                {
                    return c->get_class()->is_child_of<C>();
                }
            );
        }

        template<typename C>
        void _append(C* component)
        {
            m_components.push_back( component );
            component->init_owner(this);
        }

        std::string                            m_name;
        std::vector<Component*>                m_components;
    };
}
