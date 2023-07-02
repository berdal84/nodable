#pragma once

// libs
#include <observe/event.h>

// std
#include <string>
#include <memory>
#include <algorithm>

#include <fw/core/assertions.h>
#include <fw/core/types.h>

#include <ndbl/core/Component.h>
#include <ndbl/core/Node.h>

namespace ndbl {

    class Components
    {
    public:
        using const_iterator = std::unordered_map<size_t, Component*>::const_iterator;

        Components(Node* _owner)
            : m_owner(_owner)
        {}

        ~Components()
        { clear(); }

        inline Node* get_owner()const
        { return m_owner; }

        /**
         * Add a component to this Node
         * Check this Node has no other Component of the same type using Node::hasComponent<T>().
         * @tparam T
         * @param _component
         */
        template<typename T, typename... Args>
        T* add(Args... args)
        {
            static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");
            auto component = new T(args...);
            component->set_owner(m_owner);
            m_components.emplace(fw::type::get<T>().hash_code(), component);
            return component;
        }

        /**
         * Ask if this Node has a Component with type T.
         * @tparam T must be Component derived.
         * @return true if this node has the component specified by it's type T.
         */
        template<typename T>
        [[nodiscard]] inline bool
        has()const
        { return get<T>(); }

        /**
         * Get all components of this Node
         */
        [[nodiscard]] inline const std::unordered_map<size_t, Component*>&
        get()const
        { return m_components; }

        /**
         * Delete a component of this node by specifying its type.
         * @tparam T must be Component derived.
         */
        template<typename T>
        void remove()
        {
            static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");
            fw::type desired_class = fw::type::get<T>();
            auto component = get<T>();
            m_components.erase(desired_class.hash_code());
            delete component;
        }

        /**
         *  Get a Component by type.
         * @tparam T must be Component derived.
         * @return a T pointer.
         */
        template<typename T>
        T* get()const
        {
            static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");

            if ( m_components.empty() )
            {
                return nullptr;
            }

            fw::type desired_class = fw::type::get<T>();

            // Search with class name
            {
                auto it = m_components.find( desired_class.hash_code() );
                if (it != m_components.end())
                {
                    return static_cast<T*>(it->second);
                }
            }

            // Search for a derived class
            for (const auto & [name, component] : m_components)
            {
                if ( component->get_type().is_child_of(desired_class) )
                {
                    return static_cast<T*>(component);
                }
            }

            return nullptr;
        };

        size_t clear()
        {
            size_t count(m_components.size());
            for ( const auto& keyComponentPair : m_components)
            {
                delete keyComponentPair.second;
            }
            m_components.clear();
            return count;
        }

        inline const_iterator
        begin() const { return m_components.cbegin(); }

        inline const_iterator
        end() const { return m_components.cend(); }

    protected:
        std::unordered_map<size_t, Component*> m_components;
        Node*                                  m_owner;
    };
}
