#pragma once
#include "Component.h"
#include <unordered_map>

#define COMPONENT_BAG_FOR( COMPONENT_CLASS_NAME ) \
    COMPONENT_BAG_EX( COMPONENT_CLASS_NAME, COMPONENT_CLASS_NAME##Bag )

#define COMPONENT_BAG_EX( COMPONENT_CLASS_NAME, CLASS_NAME ) \
    class CLASS_NAME \
    {\
        typedef COMPONENT_CLASS_NAME C;\
    public:\
        typedef typename std::unordered_map<size_t, C*>::const_iterator const_iterator; \
        \
        template<typename T> const_iterator find()const { return m_components_by_id.find( get_id<T>() ); }\
        template<typename T> bool           has()const  { return find<T>() != m_components_by_id.end(); }\
        \
        const_iterator         begin() const { return m_components_by_id.cbegin(); }\
        const_iterator         end()   const { return m_components_by_id.cend(); }\
        const std::vector<C*>& get_all(){ return m_components; }\
        \
        void reset_owner(C::owner_t* owner)\
        {\
            m_owner = owner;\
            for(auto c : m_components )\
                c->reset_owner( owner );\
        }\
        \
        template<typename T> \
        void add(T* component)\
        {\
            m_components_by_id.emplace( get_id<T>(), component);\
            m_components.push_back( component );\
            component->reset_owner( m_owner );\
        }\
        \
        template<typename T> \
        void remove(T* component)\
        { \
            auto found = std::find(m_components.begin(), m_components.end(), component );\
            VERIFY(found != m_components.end(), "Component can't be found it those components");\
            m_components_by_id.erase( get_id<T>() );\
            m_components.erase(found);\
            component->reset_owner();\
        }\
        \
        template<typename T>\
        C* get()const\
        {\
            auto it = find<T>();\
            if (it != m_components_by_id.end())\
                return it->second;\
            return {};\
        }\
    private:                                                       \
        C::owner_t*                    m_owner;\
        std::vector<C*>                m_components;\
        std::unordered_map<size_t, C*> m_components_by_id;\
        \
        template<typename T>\
        static size_t get_id()\
        { \
            static_assert( std::is_base_of_v<C, T>, "T is not a component" ); \
            return std::type_index( typeid(T) ).hash_code(); \
        } \
        __COMPONENT_BAG_CUSTOM_IMPLEM

#define __COMPONENT_BAG_CUSTOM_IMPLEM( ... ) \
        __VA_ARGS__\
    }
