#pragma once

#include <string>
#include <memory>
#include <algorithm>
#include <vector>
#include <unordered_map>

#include "Signals.h"
#include "types.h"
#include "assertions.h"
#include "memory/memory.h"
#include "reflection/reflection"
#include "ComponentFor.h"

namespace tools
{
    template<class EntityT>
    struct ComponentCollection
	{
        using ComponentT     = ComponentFor<EntityT>;
        using iterator       = typename std::vector<ComponentT*>::iterator;
        using const_iterator = typename std::vector<ComponentT*>::const_iterator;

        ComponentCollection(const ComponentCollection&) = default;

        explicit ComponentCollection(EntityT* entity)
        : _entity_ptr(entity)
        {
            ASSERT(entity);
        };

        size_t size() const
        {
            return _components.size();
        }

        template<typename T>
        bool has() const
        {
            return get<T>() != nullptr;
        }

        const std::vector<ComponentT*>& components()
        {
            return _components;
        }

        template<typename T>
        T* create()
        {
            auto* c = _allocate<T>();
            _append( c );
            return c;
        }

        template<typename T, typename ...Args>
        T* create(Args...args)
        {
            auto* c = _allocate<T>(args...);
            _append( c );
            return c;
        }

        template<typename T>
        void destroy(T* component)
        {
            _remove(component);
            _deallocate<T>(component);
        }

        template<typename T>
        T* get() const
        {
            const T* c = _get_by_type<T>();
            if ( c != nullptr )
                return const_cast<T*>( c );
            return nullptr;
        }

        template<class T>
        static std::vector<T*> get_every(const std::vector<ComponentCollection*>& entities)
        {
            std::vector<T*> result;
            result.reserve( entities.size() );

            for(ComponentCollection* _entity : entities)
            {
                result.push_back(_entity->get<T>() );
            }

            return result;
        }

        template<class T>
        T* require(const char* reason) const
        {
            T* component = get<T>();
            VERIFY(component != nullptr, reason);
            return component;
        }

        iterator       begin()        { return _components.begin(); }
        iterator       end()          { return _components.end(); }
        const_iterator cbegin() const { return _components.cbegin(); }
        const_iterator cend() const   { return _components.cend(); }
    private:

        template<typename T>
        const T* _get_by_type() const
        {
            auto it = _component_by_typeid.find(std::type_index(typeid(T)));
            if ( it != _component_by_typeid.end() )
            {
                return reinterpret_cast<const T*>(it->second);
            }
            return nullptr;
        }

        template<typename T>
        const_iterator _find(T* ptr) const
        {
            return std::find(_components.begin(), _components.end(), ptr);
        }

        template<class T>
        void _append(T* c)
        {
            _components.push_back( c );
            auto it = _component_by_typeid.emplace( std::type_index(typeid(T)), c );
            ASSERT( it != _component_by_typeid.end() );
            c->set_owner(_entity_ptr);
        }

        template<class T>
        void _remove(T* c)
        {
            size_t count = _component_by_typeid.erase(c);
            ASSERT(count == 1);
            _components.erase( std::find(_components.begin(), _components.end(), c ) );
            delete c;
        }

        // for later conversion to an allocator

        template<typename T>                   T*   _allocate()              { return new T(); }
        template<typename T, typename ...Args> T*   _allocate(Args...args)   { return new T(args...); }
        template<class T>                      void _deallocate(T* ptr){ delete ptr; }

        EntityT* _entity_ptr;
        std::unordered_multimap<std::type_index, ComponentT*> _component_by_typeid;
        std::vector<ComponentT*> _components;
    };
}
