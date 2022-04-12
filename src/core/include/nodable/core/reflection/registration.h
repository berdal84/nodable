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
            type m_type;

            push(const char* _name)
            {
                using noqualifier_T      = typename std::decay<T>::type;
                using noptr_T            = typename std::remove_pointer<noqualifier_T>::type;
                size_t hash_code         = typeid(noqualifier_T).hash_code();

                m_type.m_underlying_type = hash_code;
                m_type.m_hash_code       = hash_code;
                m_type.m_name            = _name;
                m_type.m_is_pointer      = false;
                m_type.m_is_reference    = false;
                m_type.m_is_const        = false;

                typeregister::insert(m_type);
                LOG_MESSAGE("R", "New entry: %s is %s\n", _name, m_type.get_name() );

            }

            template<typename I>
            push<T>& extends()
            {
                m_type.add_parent(typeid(I).hash_code());
                return *this;
            }
        };


    };
} // namespace Nodable