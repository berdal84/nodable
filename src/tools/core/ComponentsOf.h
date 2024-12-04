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
    //
    // Handle a set of components for an entity class E
    //
    // minimalist example:
    //    struct MyEntity
    //    {
    //         ComponentsOf<MyEntity>& components() { return m_components }
    //         const ComponentsOf<MyEntity>& components() const { return m_components }
    //    private:
    //         ComponentsOf<MyEntity> m_components;
    //    }
    //
    template<typename E>
    struct ComponentsOf
	{
        using ComponentT     = ComponentFor<E>;
        using iterator       = typename std::vector<ComponentT*>::iterator;
        using const_iterator = typename std::vector<ComponentT*>::const_iterator;

        ComponentsOf(const ComponentsOf&) = default;

        explicit ComponentsOf(E* entity)
        : _entity_ptr(entity)
        {
            ASSERT(entity);
        };

        ~ComponentsOf()
        {
            if ( !_components.empty() )
            {
                LOG_WARNING("ComponentCollection", "shutdown() was not called");
                shutdown();
            }
            assert(_components.empty());
            assert(_component_by_typeid.empty());
        }

        void shutdown() // free memory
        {
            while( !_components.empty() )
            {
                ComponentT* component = _components.back();
                auto it = std::find_if(_component_by_typeid.begin(), _component_by_typeid.end(), [&](const auto& pair) { return pair.second == component; });
                VERIFY(it != _component_by_typeid.end(), "Unable to find component in the _component_by_typeid container!");
                _component_by_typeid.erase(it);
                _deallocate(component);
                _components.pop_back();
            }
            ASSERT(_component_by_typeid.empty());
        }

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
            auto it = std::find_if(_component_by_typeid.begin(), _component_by_typeid.end(), [&](const auto& pair) { return pair.second == component; });
            ASSERT(it != _component_by_typeid.end());
            _component_by_typeid.erase(it);
            _components.erase( std::find(_components.begin(), _components.end(), component ) );
            _deallocate(component);
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
        static std::vector<T*> get_every(const std::vector<ComponentsOf*>& entities)
        {
            std::vector<T*> result;
            result.reserve( entities.size() );

            for(ComponentsOf* _entity : entities)
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
            c->set_entity(_entity_ptr);
        }

        // for later conversion to an allocator

        template<typename T, typename ...Args> T*   _allocate(Args...args)   { return new T(args...); }
        template<class T>                      void _deallocate(T* ptr){ delete ptr; }

        E* _entity_ptr;
        std::unordered_multimap<std::type_index, ComponentT*> _component_by_typeid;
        std::vector<ComponentT*> _components;
    };
}
