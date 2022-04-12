#pragma once
#include <nodable/core/reflection/typeregister.h>
#include <nodable/core/reflection/type.h>

namespace Nodable
{
    class registration
    {
        template<typename T>
        struct unqualified
        {
            using  type = typename std::remove_pointer< typename std::decay<T>::type >::type;
            static const size_t hash_code() { return typeid(type).hash_code(); };
            static const char* name() { return typeid(type).name(); };
        };

        template<typename T>
        static type& configure(type& type, const char* _name)
        {
            type.m_underlying_type = unqualified<T>::hash_code();
            type.m_hash_code       = unqualified<T>::hash_code();
            type.m_name            = _name;
            type.m_is_pointer      = false;
            type.m_is_reference    = false;
            type.m_is_const        = false;

            return type;
        }

    public:


        template<typename T>
        class push
        {
        public:
            type m_type;

            push(const char* _name)
            {
                configure<T>(m_type, _name);
                typeregister::insert(m_type);
                LOG_MESSAGE("R", "registration::push() - %16llu: %s\n", m_type.m_hash_code, m_type.get_name() );
            }
        };

        template<typename T>
        class push_class
        {
        public:
            type m_type;

            push_class(const char* _name)
            {
                configure<T>(m_type, _name);

            }

            ~push_class()
            {
                typeregister::insert(m_type);
                LOG_MESSAGE("R", "registration::push_class() - %16llu: %s\n", m_type.m_hash_code, m_type.get_name() );
            }

            template<typename I>
            push_class& extends()
            {
                // todo: ensure parent exists
                using U = unqualified<I>;
                if( !typeregister::has( U::hash_code() ) )
                {
                    type t;
                    configure<typename U::type>(t, U::name() );
                    typeregister::insert(t);
                }

                m_type.add_parent( U::hash_code() );
                return *this;
            }

        };
    };
} // namespace Nodable