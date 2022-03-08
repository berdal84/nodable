#pragma once

/*
 *
 * Tiny replacement for Groufon's mirror library to work with c++11
 *
 * Objective:
 *
 * - type base types (string, double, bool, etc.)
 * - ask is_child_of (for classes)
 * - get/set members (TODO)
 * - automatic R_INHERITS declaration (TODO)
 *
 * How to use ?
 *
 * - include this header file, dependent headers are included here.
 *
 */

#include <string> // for std::string reflection
#include <map>
#include <typeinfo>
#include <memory> // std::shared_ptr
#include <nodable/Log.h>

/* internal */
#include <nodable/R_internal_Class.h>
#include <nodable/R_internal_Type.h>
#include <nodable/R_internal_Typename.h>
#include <nodable/R_internal_Meta.h>

#define ENABLE_ANY_PTR_TO_BE_CONSIDERED_VOID_PTR true

namespace Nodable::R
{
    // MACROS are written without namespace
    #include <nodable/R_internal_Class_MACROS.h>
    #include <nodable/R_internal_Type_MACROS.h>

    using unknown_t = std::nullptr_t; // <----- seriously doubting about this idea.

    /**
     * Initialize R.
     * - register base types
     * - log statistics
     */
    void init();

    void log_statistics();

    /**
     * Static structure to store some register.
     */
    struct TypeRegister
    {
        static std::map<Typename, std::shared_ptr<const Type>>&    by_enum();
        static std::map<std::string, std::shared_ptr<const Type>>& by_typeid();
        static bool has_typeid(const std::string&);

    private:
        /** push template */
        template<typename T, bool is_class> struct _push;

        /** push a class */
        template<typename T>
        struct _push<T, true> {
            _push()
            {
                std::string id = typeid(T).name();
                if ( !TypeRegister::has_typeid(id) )
                {
                    Type_ptr type = T::Get_class();
                    TypeRegister::by_typeid()[id] = type;
                    LOG_MESSAGE("R", "New entry: %s is %s\n", type->get_name(), to_string(type->get_typename()) );
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
                if ( !TypeRegister::has_typeid(id) )
                {
                    Type_ptr type;
                    const Typename reflect_t = meta_type<T>::reflect_t;
                    type = std::make_shared<meta_enum<reflect_t>>();
                    TypeRegister::by_enum()[reflect_t] = type;

                    TypeRegister::by_typeid()[id] = type;
                    LOG_MESSAGE("R", "New entry: %s is %s\n", type->get_name(), to_string(type->get_typename()) );
                }
             }
        };

    public:
        /** detect if we push a class or a regular type */
        template<typename T, bool is_class = std::is_class_v<T> && !std::is_same_v<T, std::string>>
        struct push : _push<T, is_class> {};
    };

    /* get meta for a specific type at runtime */
    static std::shared_ptr<const Type> get_type(Typename t)
    {
        auto found = TypeRegister::by_enum().find(t);
        if (found != TypeRegister::by_enum().end() )
            return found->second;
        return nullptr;
    };

    /* get meta for a specific type at runtime */
    template<typename T>
    static std::shared_ptr<const Type> get_type()
    {
        if( std::is_pointer<T>::value )
        {
            std::shared_ptr<const Type> base = get_type<std::remove_pointer_t<T>>();
            return Type::make_ptr( base );
        }
        else if( std::is_reference<T>::value )
        {
            std::shared_ptr<const Type> base = get_type<std::remove_reference_t<T>>();
            return Type::make_ref( base );
        }

        std::string id = typeid(T).name();
        auto found = TypeRegister::by_typeid().find(id);
        if ( found != TypeRegister::by_typeid().end() )
        {
            return found->second;
        }
        return nullptr;

    };

    /*
     * Match some cpp types to Type enum.
     */
    R_LINK_TYPE(double       , Typename::Double )
    R_LINK_TYPE(std::string  , Typename::String )
    R_LINK_TYPE(bool         , Typename::Boolean )
    R_LINK_TYPE(void         , Typename::Void )

    template<typename T>
    class meta_type<T*> : public meta_type<T> {};

    template<typename T>
    class meta_type<T&> : public meta_type<T> {};

}

