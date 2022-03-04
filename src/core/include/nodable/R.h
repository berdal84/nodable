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
#include <nodable/Log.h>

/* internal */
#include <nodable/R_internal_Class.h>
#include <nodable/R_internal_Type.h>
#include <nodable/R_internal_Typename.h>

#define ENABLE_ANY_PTR_TO_BE_CONSIDERED_VOID_PTR true

namespace Nodable::R
{
    // MACROS are written without namespace
    #include <nodable/R_internal_Class_MACROS.h>
    #include <nodable/R_internal_Type_MACROS.h>

    using unknown_t = std::nullptr_t; // <----- seriously doubting about this idea.

    /**
     * Static structure to store some register.
     */
    struct Register
    {
        /**
         * Get Types by Typename
         * @return
         */
        static std::map<Typename, Type*>& by_enum();

        /**
         * Get Types by typeid
         * @return
         */
        static std::map<std::string, Type*>& by_typeid();
    };

    static void LogStats()
    {
        LOG_MESSAGE("R", "Logging reflected types ...\n");

        LOG_MESSAGE("R", "By typename (%i):\n", Register::by_enum().size() );
        for ( auto each : Register::by_enum() )
        {
            LOG_MESSAGE("R", " Typename::%s => %s \n", to_string(each.first), each.second->get_name() );
        }

        LOG_MESSAGE("R", "By typeid (%i):\n", Register::by_enum().size() );
        for ( auto each : Register::by_typeid() )
        {
            LOG_MESSAGE("R", " %s => %s \n", each.first.c_str(), each.second->get_name() );
        }

        LOG_MESSAGE("R", "Logging done.\n");
    }

    /* get meta for a specific type at runtime */
    template<typename T>
    static const Type* get_type()
    {
        if( std::is_pointer<T>::value )
        {
            const Type* base = get_type<std::remove_pointer_t<T>>();
            return Type::make_ptr( base );
        }
        else if( std::is_reference<T>::value )
        {
            const Type* base = get_type<std::remove_reference_t<T>>();
            return Type::make_ref( base );
        }
        std::string id = typeid(T).name();
        auto found = Register::by_typeid().find(id);
        if ( found != Register::by_typeid().end() )
        {
            return found->second;
        }
        return nullptr;

    };

    /* get meta for a specific type at runtime */
    static const Type* get_meta(Typename t)
    {
        auto found = Register::by_enum().find(t);
        if ( found != Register::by_enum().end() )
            return found->second;
        return nullptr;
    };

   /**
    * Struct to insert a meta_type
    * @tparam CPP_T
    * @tparam REFLECT_T
    */
    template<typename CPP_T, Typename REFLECT_T>
    struct register_type
    {
        register_type()
        {
            if ( !get_meta(REFLECT_T) )
            {
                Type* type      = new meta_enum<REFLECT_T>();
                std::string id  = typeid(CPP_T).name();
                Register::by_enum()[REFLECT_T] = type;
                Register::by_typeid()[id]      = type;

                LOG_MESSAGE("R", "New entry: %s is %s\n", type->get_name(), to_string(type->get_typename()) );
            }
        };
    };

    template<typename T>
    struct register_class
    {
        register_class(const char* _name)
        {
            if ( !get_type<T>() )
            {
                Type* type     = new Class(_name);
                std::string id = typeid(T).name();

                Register::by_typeid()[id] = type;

                LOG_MESSAGE("R", "New entry: %s is %s\n", type->get_name(), to_string(type->get_typename()) );
            }
        };
    };

    /* just log the reflected types */
    static void Initialize()
    {
        LogStats();
    }

    /*
     * Match some cpp types to Type enum.
     */
    R_DEF_TYPENAME(double       , Typename::Double )
    R_DEF_TYPENAME(std::string  , Typename::String )
    R_DEF_TYPENAME(bool         , Typename::Boolean )
    R_DEF_TYPENAME(void         , Typename::Void )

#if ENABLE_ANY_PTR_TO_BE_CONSIDERED_VOID_PTR
/*
 * To avoid the ned to overdeclare, this templae allows any object pointer to be treater as a void*
 * ex: Node*, Object*, Car*, Character* will be treated as void*
 */
     template<typename T>
     class meta_type<T*> : public meta_type<T> {};

    template<typename T>
    class meta_type<T&> : public meta_type<T> {};
#endif

}

