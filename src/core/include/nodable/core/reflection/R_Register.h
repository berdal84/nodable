#pragma once

#include <string>
#include <map>
#include <nodable/core/Log.h>
#include <type_traits>
#include "R_Type.h"
#include "R_Meta_t.h"

namespace Nodable { namespace R
{
    template<typename T>
    struct is_class
    {
        constexpr static bool value = std::is_class<T>::value && !std::is_same<T, std::string>::value;
    };

    /**
     * Static structure to store some register.
     */
    struct Register
    {
        static std::map<Type, std::shared_ptr<const Meta_t>>& by_type();
        static std::map<std::string, std::shared_ptr<const Meta_t>>&  by_typeid();
        static bool has_typeid(const std::string&);
        template<class T> constexpr static bool has_class()
        {
            return std::is_member_function_pointer<decltype(&T::get_class)>::value;
        }
    private:
        /** push template */
        template<typename T, bool is_class> struct _push;

        /** push a class */
        template<typename T>
        struct _push<T, true> {
            _push()
            {
                std::string id = typeid(T).name();
                if ( !Register::has_typeid(id) )
                {
                    Meta_t_csptr meta_type = T::Get_class();
                    Register::by_typeid()[id] = meta_type;
                    LOG_MESSAGE("R", "New entry: %s is %s\n", meta_type->get_name(), to_string(meta_type->get_type()) );
                }
            }
        };

        /** Push a non class */
        template<typename T>
        struct _push<T, false>
        {
            _push()
            {
                std::string id = typeid(T).name();
                if ( !Register::has_typeid(id) )
                {
                    Meta_t_csptr meta_type = std::make_shared<Meta_t>(reflect_t<T>::name, reflect_t<T>::type_v);
                    Register::by_type().insert({meta_type->get_type(), meta_type});
                    Register::by_typeid().insert({id, meta_type});
                    LOG_MESSAGE("R", "New entry: %s is %s\n", meta_type->get_name(), to_string(meta_type->get_type()) );
                }
            }
        };

    public:

        /** detect if we push a class or a regular type */
        template<typename T, bool is_class = is_class<T>::value>
        struct push : _push<T, is_class> {};
    };
} }