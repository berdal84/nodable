#pragma once
#include <nodable/core/reflection/typeregister.h>
#include <nodable/core/reflection/type.h>

namespace Nodable
{
    struct registration
    {
        template<typename T>
        struct push
        {
            type t;

            push(const char* _name): t(typeid(T).hash_code(), _name)
            {
                t.m_is_pointer    = std::is_pointer<T>();
                t.m_is_reference  = std::is_reference<T>();
                t.m_is_const      = std::is_const<T>();

                typeregister::insert(t);
                LOG_MESSAGE("R", "New entry: %s is %s\n", _name, t.get_name() );
            }

            template<typename I>
            push<T>& extends()
            {
                t.add_parent(typeid(I).hash_code());
                return *this;
            }
        };


    };
} // namespace Nodable