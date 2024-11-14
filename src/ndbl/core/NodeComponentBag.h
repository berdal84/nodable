#pragma once
#include "NodeComponent.h"
#include <unordered_map>

namespace ndbl
{
    class NodeComponentBag
    {
    public:
        typedef std::unordered_map<size_t, NodeComponent*>::const_iterator const_iterator;

        template<typename T>
        const_iterator find()const
        { return m_components_by_id.find( get_id<T>() ); }

        template<typename T> bool
        has()const
        { return find<T>() != m_components_by_id.end(); }

        const_iterator begin() const { return m_components_by_id.cbegin(); }
        const_iterator end()   const { return m_components_by_id.cend(); }

        const std::vector<NodeComponent*>& get_all(){ return m_components; }

        void reset_owner(Node* owner)
        {
            m_owner = owner;
            for(auto c : m_components )
                c->reset_owner( owner );
        }

        template<typename T>
        void add(T* component)
        {
            m_components_by_id.emplace( get_id<T>(), component);
            m_components.push_back( component );
            component->reset_owner( m_owner );
        }

        template<typename T>
        void remove(T* component)
        {
            auto found = std::find(m_components.begin(), m_components.end(), component );
            VERIFY(found != m_components.end(), "Component can't be found it those components");
            m_components_by_id.erase( get_id<T>() );
            m_components.erase(found);
            component->reset_owner();
        }

        template<typename T>
        NodeComponent* get()const
        {
            auto it = find<T>();
            if (it != m_components_by_id.end())
                return it->second;
            return {};
        }
    private:
        Node*                                      m_owner;
        std::vector<NodeComponent*>                m_components;
        std::unordered_map<size_t, NodeComponent*> m_components_by_id;

        template<typename T>
        static size_t get_id()
        {
            static_assert( std::is_base_of_v<NodeComponent, T>, "T is not a component" );
            return std::type_index( typeid(T) ).hash_code();
        }
    };
}