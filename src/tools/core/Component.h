#pragma once

#include <string>
#include <algorithm>
#include <vector>

#include "Signals.h"
#include "Asserts.h"
#include "core/reflection/Type_Descriptor.h"

namespace tools
{
    template<typename T>
    struct Component_Bag;

    //
    // Base struct to implement a new Component for a given Entity_Type
    //
    template<typename Entity_Type>
    requires std::is_object_v<Entity_Type>
    struct Component
    {
        tools::Simple_Signal            signal_init;            // called after component knows its entity
        tools::Simple_Signal            signal_shutdown;        // called before to be deleted, when component still knows its entity
        Entity_Type*                    entity      = nullptr;
        const tools::Type_Descriptor*   type_desc   = nullptr;
        const char*                     name        = "";

        Component(const char* _name)
        : name(_name)
        {}
        
        virtual ~Component() = default; // TODO: Do I really need this virtual destructor? Yes, until we call delete on Component<Entity_Type> pointers.
    };

    
    template<typename Entity_Type>
    void component_init(Component<Entity_Type>* component, Entity_Type* entity, const Type_Descriptor* type_desc = nullptr)
    {
        VERIFY(entity != nullptr, "Entity is required");

        if( type_desc == nullptr )
        {
            component->type_desc = type::get<Component<Entity_Type>>();
            ASSERT(component->type_desc != nullptr);
        }
        else
        {
            component->type_desc = type_desc;
        }

        component->entity = entity;
        component->signal_init.emit();
        TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Component", "_init \"%s\" (type: %s ) ...\n", component->name, component->type_desc->name() );
    }

    template<typename Entity_Type>
    void component_shutdown(Component<Entity_Type>* component) // do the mirror of component_init()
    {
        TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Component", "_shutdown \"%s\" ...\n", component->name);
        component->signal_shutdown.emit();
        component->entity    = nullptr;
        component->type_desc = nullptr;
    }

    template<typename T, typename Entity_Type>
    concept Component_For = requires (T t) {
        std::is_base_of_v<Component<Entity_Type>, T>;
    };

    //
    // Component_Bag:
    //      Handle a set of components for an entity class Entity_Type
    //
    // Minimalist example with components having a default constructor:
    //    struct MyEntity
    //    {
    //         template<typename T>   create_component() { return _m_components.create<T>(); }
    //         template<typename T>   get_component()    { return _m_components.get<T>(); }
    //    private:
    //         Component_Bag<MyEntity> _m_components;
    //    }
    //
    template<typename Entity_Type>
    struct Component_Bag
	{
        using ComponentT     = Component<Entity_Type>;
        using iterator       = typename std::vector<ComponentT*>::iterator;
        using const_iterator = typename std::vector<ComponentT*>::const_iterator;
        using ComponentByTypeIndex = std::unordered_multimap<std::type_index, ComponentT*>;
//====== Data ==========================================================================================================
    private:
        ComponentByTypeIndex     _m_component_indexed_by_typeid;
        std::vector<ComponentT*> _m_component;
        Entity_Type*                 entity;
//====== Methods =======================================================================================================
    public:
        Component_Bag() = delete;
        explicit Component_Bag(Entity_Type* entity)
        : entity(entity)
        {
            ASSERT(entity);
        };
        Component_Bag(const Component_Bag&) = delete;
        Component_Bag(Component_Bag&&) = delete;

        ~Component_Bag()
        {
            assert(_m_component.empty()); // did you called shutdown() before to delete?
            assert(_m_component_indexed_by_typeid.empty()); // should be empty if _m_component is.
        }

        void shutdown() noexcept // free memory
        {
            // TODO: we could optimize these two loops by iterating once.
            //       but for some reasons components have unordered dependencies that needs to be fixed.
            for(ComponentT* component : _m_component)
                component_shutdown(component);
            for(ComponentT* component : _m_component)
                _deallocate(component);
            _m_component.clear();
            _m_component_indexed_by_typeid.clear();
        }

        size_t size() const
        {
            return _m_component.size();
        }

        template<Component_For<Entity_Type> T>
        bool has() const
        {
            return get<T>() != nullptr;
        }

        const std::vector<ComponentT*>& components()
        {
            return _m_component;
        }

        template<Component_For<Entity_Type> T>
        T* create()
        {
            auto* c = _allocate<T>();
            _append( c );
            return c;
        }

        template<Component_For<Entity_Type> T, typename ...Args>
        T* create(Args...args)
        {
            auto* c = _allocate<T>(args...);
            _append( c );
            return c;
        }

        template<Component_For<Entity_Type> T>
        void destroy(T* component)
        {
            auto it = std::find_if(_m_component_indexed_by_typeid.begin(), _m_component_indexed_by_typeid.end(), [&](const auto& pair) { return pair.second == component; });
            ASSERT(it != _m_component_indexed_by_typeid.end());
            _m_component_indexed_by_typeid.erase(it);
            _m_component.erase(std::find(_m_component.begin(), _m_component.end(), component ) );
            component_shutdown(component);
            _deallocate(component);
        }

        template<Component_For<Entity_Type> T>
        T* get() const
        {
            const T* c = _get_by_type<T>();
            if ( c != nullptr )
                return const_cast<T*>( c );
            return nullptr;
        }

        template<Component_For<Entity_Type> T>
        static std::vector<T*> get_every(const std::vector<Component_Bag*>& entities)
        {
            std::vector<T*> result;
            result.reserve( entities.size() );

            for(Component_Bag* _entity : entities)
            {
                result.push_back(_entity->get<T>() );
            }

            return result;
        }

        template<Component_For<Entity_Type> T>
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

        template<Component_For<Entity_Type> T>
        const T* _get_by_type() const
        {
            auto it = _m_component_indexed_by_typeid.find(std::type_index(typeid(T)));
            if (it != _m_component_indexed_by_typeid.end() )
            {
                return reinterpret_cast<const T*>(it->second);
            }
            return nullptr;
        }

        template<Component_For<Entity_Type> T>
        const_iterator _find(T* ptr) const
        {
            return std::find(_m_component.begin(), _m_component.end(), ptr);
        }

        template<Component_For<Entity_Type> T>
        void _append(T* component)
        {
            _m_component.push_back(component );
            const auto* type = type::get<T>();
            auto it = _m_component_indexed_by_typeid.emplace( type->id() , component );
            ASSERT(it != _m_component_indexed_by_typeid.end() );
            component_init(component, entity, type );
        }

        // for later conversion to an allocator

        template<typename T, typename ...Args> T*   _allocate(Args...args)   { return new T(args...); }
        template<class T>                      void _deallocate(T* ptr){ delete ptr; }
    };
}
