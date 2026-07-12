#pragma once

#include <cstddef>
#include <string>
#include <algorithm>
#include <typeindex>
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
    template<typename _Entity_Type>
    requires std::is_object_v<_Entity_Type>
    struct Component
    {
        using Entity_Type = _Entity_Type;

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

    
    template<
        typename Component_Type,
        typename Entity_Type = Component_Type::Entity_Type
    >
    void component_init(Component_Type* component, Entity_Type* entity)
    {
        VERIFY(entity != nullptr, "Entity is required");

        component->type_desc = type::get<Component_Type>();
        component->entity    = entity;
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
    //         template<typename T>   create_component() { return components.create<T>(); }
    //         template<typename T>   get_component()    { return components.get<T>(); }
    //    private:
    //         Component_Bag<MyEntity> components;
    //    }
    //
    template<typename Entity_Type>
    struct Component_Bag
	{
        using Component_Type    = Component<Entity_Type>;
        using iterator          = typename std::vector<Component_Type*>::iterator;
        using const_iterator    = typename std::vector<Component_Type*>::const_iterator;

        Entity_Type*                 entity     = nullptr; // TODO: I could remove this field, we don't use it.
        std::vector<Component_Type*> components = {};
        std::unordered_multimap<std::type_index, Component_Type*> components_indexed_by_typeid;

        Component_Bag() = default;
        Component_Bag(const Component_Bag&) = delete;
        Component_Bag(Component_Bag&&)      = delete;

        ~Component_Bag()
        {
            assert(components.empty()); // did you called shutdown() before to delete?
            assert(components_indexed_by_typeid.empty()); // should be empty if _m_component is.
        }

        inline iterator         begin()        { return components.begin(); }
        inline iterator         end()          { return components.end(); }
        inline const_iterator   cbegin() const { return components.cbegin(); }
        inline const_iterator   cend() const   { return components.cend(); }
        
        inline size_t           size() const   { return components.size(); }
        inline bool             empty() const  { return components.size() == 0; }
    };
    
    template<typename Entity_Type>
    void componentbag_clear(Component_Bag<Entity_Type>* bag)
    {
        bag->components.clear();
        bag->components_indexed_by_typeid.clear();
    }

    template<typename Entity_Type>
    void componentbag_init(Component_Bag<Entity_Type>* bag, Entity_Type* entity)
    {
        VERIFY(entity != nullptr, "An entity is required");
        VERIFY(bag->entity == nullptr, "Cannot initialize twice");        
        bag->entity = entity;
    }

    template<typename Entity_Type>
    void componentbag_shutdown(Component_Bag<Entity_Type>* bag)
    {
        VERIFY(bag->entity != nullptr, "This Component_Bag was not initialized");
        componentbag_clear(bag);
        bag->entity = nullptr;
    }    

    template<
        typename Component_Type,
        typename Entity_Type = typename Component_Type::Entity_Type
    >
    Component_Type* componentbag_get(Component_Bag<Entity_Type>* bag)
    {
        std::type_index type_index = std::type_index(typeid(Component_Type));
        auto it = bag->components_indexed_by_typeid.find(type_index);
        if (it != bag->components_indexed_by_typeid.end() )
        {
            return reinterpret_cast<Component_Type*>(it->second);
        }
        return nullptr;
    }

    template<
        typename Component_Type,
        typename Entity_Type = typename Component_Type::Entity_Type
    >
    bool componentbag_has(const Component_Bag<Entity_Type>* bag)
    {
        return componentbag_get<Component_Type>(bag) != nullptr;
    }

    template<
        typename Component_Type,
        typename Entity_Type = typename Component_Type::Entity_Type
    >
    void componentbag_remove(Component_Bag<Entity_Type>* bag, Component_Type* component)
    {
        ASSERT(bag->entity != nullptr); // This Component_Bag was not initialized
        VERIFY(component->entity == bag->entity, "This component does not belong to the same entity or call component_shutdown() after componentbag_remove()");

        // erase from indexed_by_typeid
        auto it = std::find_if(
            bag->components_indexed_by_typeid.begin(), bag->components_indexed_by_typeid.end(),
            [&](const auto& pair) { return pair.second == component; }
        );
        ASSERT(it != bag->components_indexed_by_typeid.end());
        bag->components_indexed_by_typeid.erase(it);

        // erase
        bag->components.erase(
            std::find(bag->components.begin(), bag->components.end(), component )
        );        
    }

    template<
        typename Component_Type,
        typename Entity_Type = typename Component_Type::Entity_Type
    >
    void componentbag_add(Component_Bag<Entity_Type>* bag, Component_Type* component)
    {
        ASSERT(bag->entity != nullptr); // This Component_Bag was not initialized
        VERIFY(component->entity == bag->entity, "This component does not belong to the same entity or call component_init() first");

        // add to index
        const auto* type_desc = type::get<Component_Type>();
        auto it = bag->components_indexed_by_typeid.emplace( type_desc->id() , component );
        ASSERT(it != bag->components_indexed_by_typeid.end() );
        
        // add
        bag->components.push_back(component );        
    }
}
