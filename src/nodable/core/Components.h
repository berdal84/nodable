#pragma once
#include "fw/core/reflection/reflection"
#include "fw/core/Pool.h"
#include <unordered_map>

namespace ndbl {

    // forward declarations
    class Node;
    class Component;
    using fw::pool::ID;

    /**
     * Store a list of Components* owned by a single owner.
     * Components* are not owned by this class, see ComponentManager.
     */
    class Components
    {
        friend Node;
    public:

        Components() = default;

        void              set_owner(ID<Node>);
        void              add(ID<Component>);
        void              remove(ID<Component>);

        /**
         * Ask if this Node has a Component with type T.
         * @tparam T must be Component derived.
         * @return true if this node has the component specified by it's type T.
         */
        template<typename T>
        [[nodiscard]] inline bool
        has()const
        { return get<T>() != fw::pool::ID_NULL; }

        template<typename T>
        ID<T> get()const;

        inline std::unordered_map<fw::type::id_t , ID<Component>>::const_iterator
        begin() const { return m_components_by_type.cbegin(); }

        inline std::unordered_map<fw::type::id_t, ID<Component>>::const_iterator
        end() const { return m_components_by_type.cend(); }

        const std::vector<ID<Component>>& get_all()
        { return m_components; }

    private:
        ID<Node> m_owner;
        std::vector<ID<Component>> m_components;
        std::unordered_map<fw::type::id_t, ID<Component>> m_components_by_type;
    };

    template<typename T>
    ID<T>
    Components::get() const
    {
        static_assert(fw::is_base_of<Component, T>::value, "ComponentT must inherit from Component");
        if ( m_components_by_type.empty() ) return {};

        auto desired_typeid = std::type_index(typeid(T));

        // Search with class name
        {
            auto it = m_components_by_type.find( desired_typeid );
            if (it != m_components_by_type.end())
            {
                return ID<T>{it->second};
            }
        }

        return {};
    }
}
