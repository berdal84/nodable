#pragma once
#include <nodable/core/reflection/database.h>
#include <nodable/core/reflection/type.h>

namespace Nodable
{
    struct registration
    {
        template<typename T>
        struct push
        {
            push(const char* _name)
            {
                size_t hash = typeid(T).hash_code();
                type t(hash, _name);
                database::insert(t);
                LOG_MESSAGE("R", "New entry: %s is %s\n", _name, t.get_name() );
            }
        };
    };
} // namespace Nodable