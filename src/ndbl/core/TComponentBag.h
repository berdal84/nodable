#pragma once
#include "TComponent.h"
#include <unordered_map>

namespace ndbl
{
     /// Store a list of Components* owned by a single owner.
     /// Components* are not owned by this class, see ComponentManager.
     /// \tparam OwnerT the owner's type
     /// \tparam ComponentT the component's base type
    template<typename OwnerT, typename ComponentT>
    class TComponentBag
    {
    private:
        OwnerT m_owner;
        std::vector<ComponentT> m_components;
        std::unordered_map<size_t, ComponentT> m_components_by_type;
    public:
        using ConstIterator = typename std::unordered_map<size_t, ComponentT>::const_iterator;

        TComponentBag() = default;

        void set_owner(OwnerT owner)
        {
            m_owner = owner;
            for(auto each_component : m_components )
            {
                each_component->set_owner( owner );
            }
        }

        template<typename T>
        void add(T component)
        {
            static_assert( std::is_assignable_v<T, ComponentT> );

            auto index =  std::type_index(typeid(T)).hash_code();
            m_components_by_type.emplace( index, component );
            m_components.push_back(component );
            component->set_owner( m_owner );
        }

        template<typename T>
        void remove(T component)
        {
            static_assert( std::is_assignable_v<T, ComponentT> );

            auto found = std::find(m_components.begin(), m_components.end(), component );
            EXPECT(found != m_components.end(), "Component can't be found it those components");
            auto index =  std::type_index( typeid(T) ).hash_code();
            m_components_by_type.erase( index );
            m_components.erase(found);
            component->set_owner({});
        }

        template<typename T>
        inline bool has()const
        { return find<T>() != m_components_by_type.end(); }

        template<typename T>
        ConstIterator find()const
        {
            auto index = std::type_index(typeid(T)).hash_code();
            return m_components_by_type.find( index );
        }

        template<typename T>
        T get()const
        {
            auto it = find<T>();
            if (it != m_components_by_type.end())
            {
                return {it->second};
            }
            return {};
        }

        inline ConstIterator begin() const
        { return m_components_by_type.cbegin(); }

        inline ConstIterator end() const
        { return m_components_by_type.cend(); }

        inline const std::vector<ComponentT>& get_all()
        { return m_components; }
    };
}
