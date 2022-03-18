#pragma once

#include <string>
#include <map>
#include <nodable/core/Log.h>
#include "R_Type.h"
#include "R_MetaType.h"

namespace Nodable::R
{
    template<typename T>
    static constexpr bool is_class = std::is_class_v<T> && !std::is_same_v<T, std::string>;

    /**
     * Static structure to store some register.
     */
    struct Register
    {
        static std::map<Type, std::shared_ptr<const MetaType>>& by_type();
        static std::map<std::string, std::shared_ptr<const MetaType>>&  by_typeid();
        static bool has_typeid(const std::string&);
        template<class T> constexpr static bool has_class()
        {
            return std::is_member_function_pointer_v<decltype(&T::get_class)>;
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
                    MetaType_const_ptr meta_type = T::Get_class();
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
                    MetaType_const_ptr meta_type = reflect_type<T>::new_meta_type();
                    Register::by_type().insert({meta_type->get_type(), meta_type});
                    Register::by_typeid().insert({id, meta_type});
                    LOG_MESSAGE("R", "New entry: %s is %s\n", meta_type->get_name(), to_string(meta_type->get_type()) );
                }
            }
        };

    public:

        /** detect if we push a class or a regular type */
        template<typename T, bool is_class = is_class<T>>
        struct push : _push<T, is_class> {};
    };
}