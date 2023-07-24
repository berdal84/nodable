#pragma once
#include "fw/core/reflection/reflection"
#include <unordered_map>

namespace ndbl {

    // forward declarations
    class Node;
    class Component;

    /**
     * Store a list of Components* owned by a single owner.
     * Components* are not owned by this class, see ComponentManager.
     */
    class Components
    {
    public:
        Components(Node* _owner)
            : m_owner(_owner)
        {}

        inline Node* get_owner()const
        { return m_owner; }

        void       add(Component* component);
        void       remove(Component* component);
        Component* get(const fw::type*) const;

        /**
         * Ask if this Node has a Component with type T.
         * @tparam T must be Component derived.
         * @return true if this node has the component specified by it's type T.
         */
        template<typename T>
        [[nodiscard]] inline bool
        has()const
        { return get<T>(); }

        template<typename T>
        T* get()const;

        inline std::unordered_map<fw::type::id_t , Component*>::const_iterator
        begin() const { return m_components_by_type.cbegin(); }

        inline std::unordered_map<fw::type::id_t, Component*>::const_iterator
        end() const { return m_components_by_type.cend(); }

        std::vector<Component*> get_all();

    protected:
        Node*                   m_owner;
        std::vector<Component*> m_components;
        std::unordered_map<fw::type::id_t, Component*> m_components_by_type;
    };

    template<typename ComponentT>
    ComponentT*
    Components::get() const
    {
        static_assert(fw::is_base_of<Component, ComponentT>::value, "ComponentT must inherit from Component");

        if ( m_components_by_type.empty() )
        {
            return nullptr;
        }

        const fw::type* desired_class = fw::type::get<ComponentT>();
        return static_cast<ComponentT*>(get(desired_class));
    }
}
