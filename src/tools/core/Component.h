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

namespace tools
{
    template<typename T>
    struct ComponentBag;

    //
    // Class to extend to create a component for the entity of class EntityT
    // see ComponentsOf<class EntityT>
    //
    template<typename EntityT>
    class Component
    {
        friend class ComponentBag<EntityT>;
    public:
        DECLARE_REFLECT_virtual

        Component() = delete;
        Component(const char* name)
        : _m_name(name)
        , _m_entity(nullptr)
        {}
        virtual ~Component() = default;

        EntityT* entity() const
        {
            return _m_entity;
        }

        const std::string&  name() const
        {
            return _m_name;
        }

        void set_name(const std::string& name)
        {
            _m_name = name;
            signal_name_change.emit(_m_name );
        }
//====== Signals (to be connected or not by the implementation) ========================================================
    protected:
        tools::SimpleSignal                     signal_init; // called after instantiation, once component knows its entity
        tools::SimpleSignal                     signal_shutdown; // called before to be deleted, when component still knows its entity
        tools::Signal<void(const std::string&)> signal_name_change;
//====== Internal================================== ====================================================================
    private:
        void _init(EntityT* entity)
        {
            LOG_VERBOSE("Component", "_init \"%s\" ...\n", _m_name.c_str());
            _m_entity = entity;
            signal_init.emit();
        }

        void _shutdown() // do the mirror of _init()
        {
            LOG_VERBOSE("Component", "_shutdown \"%s\" ...\n", _m_name.c_str());
            signal_shutdown.emit();
            _m_entity = nullptr;
        }

        EntityT*    _m_entity;
        std::string _m_name;
    };

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
    struct ComponentBag
	{
        using ComponentT     = Component<E>;
        using iterator       = typename std::vector<ComponentT*>::iterator;
        using const_iterator = typename std::vector<ComponentT*>::const_iterator;

        ComponentBag(const ComponentBag&) = default;

        explicit ComponentBag(E* entity)
        : _entity_ptr(entity)
        {
            ASSERT(entity);
        };

        ~ComponentBag()
        {
            assert(_m_component.empty()); // did you called shutdown() before to delete?
            assert(_m_component_indexed_by_typeid.empty()); // should be empty if _m_component is.
        }

        void shutdown() noexcept // free memory
        {
            for(ComponentT* component : _m_component)
            {
                component->_shutdown();
            }
            for(ComponentT* component : _m_component)
            {
                _deallocate(component);
            }
            _m_component.clear();
            _m_component_indexed_by_typeid.clear();
        }

        size_t size() const
        {
            return _m_component.size();
        }

        template<typename T>
        bool has() const
        {
            return get<T>() != nullptr;
        }

        const std::vector<ComponentT*>& components()
        {
            return _m_component;
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
            auto it = std::find_if(_m_component_indexed_by_typeid.begin(), _m_component_indexed_by_typeid.end(), [&](const auto& pair) { return pair.second == component; });
            ASSERT(it != _m_component_indexed_by_typeid.end());
            _m_component_indexed_by_typeid.erase(it);
            _m_component.erase(std::find(_m_component.begin(), _m_component.end(), component ) );
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
        static std::vector<T*> get_every(const std::vector<ComponentBag*>& entities)
        {
            std::vector<T*> result;
            result.reserve( entities.size() );

            for(ComponentBag* _entity : entities)
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

        iterator       begin()        { return _m_component.begin(); }
        iterator       end()          { return _m_component.end(); }
        const_iterator cbegin() const { return _m_component.cbegin(); }
        const_iterator cend() const   { return _m_component.cend(); }
    private:

        template<typename T>
        const T* _get_by_type() const
        {
            auto it = _m_component_indexed_by_typeid.find(std::type_index(typeid(T)));
            if (it != _m_component_indexed_by_typeid.end() )
            {
                return reinterpret_cast<const T*>(it->second);
            }
            return nullptr;
        }

        template<typename T>
        const_iterator _find(T* ptr) const
        {
            return std::find(_m_component.begin(), _m_component.end(), ptr);
        }

        template<class T>
        void _append(T* c)
        {
            _m_component.push_back(c );
            auto it = _m_component_indexed_by_typeid.emplace(std::type_index(typeid(T)), c );
            ASSERT(it != _m_component_indexed_by_typeid.end() );
            c->_init(_entity_ptr);
        }

        // for later conversion to an allocator

        template<typename T, typename ...Args> T*   _allocate(Args...args)   { return new T(args...); }
        template<class T>                      void _deallocate(T* ptr){ delete ptr; }

        E* _entity_ptr;
        std::unordered_multimap<std::type_index, ComponentT*> _m_component_indexed_by_typeid;
        std::vector<ComponentT*> _m_component;
    };
}
