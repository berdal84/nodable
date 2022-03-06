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
     * Static structure to store some register.
     */
    struct Register
    {
        static std::map<Typename, std::shared_ptr<const Type>>& by_enum();
        static std::map<std::string, std::shared_ptr<const Type>>& by_typeid();
    };

    static void LogStats()
    {
        LOG_MESSAGE("R", "Logging reflected types ...\n");

        LOG_MESSAGE("R", "By typename (%i):\n", Register::by_enum().size() );
        for ( auto each : Register::by_enum() )
        {
            LOG_MESSAGE("R", " Typename::%s => %s \n", to_string(each.first), each.second->get_name() );
        }

        LOG_MESSAGE("R", "By typeid (%i):\n", Register::by_typeid().size() );
        for ( auto each : Register::by_typeid() )
        {
            LOG_MESSAGE("R", " %s => %s \n", each.first.c_str(), each.second->get_name() );
        }

        LOG_MESSAGE("R", "Logging done.\n");
    }

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
        auto found = Register::by_typeid().find(id);
        if ( found != Register::by_typeid().end() )
        {
            return found->second;
        }
        return nullptr;

    };

    /* get meta for a specific type at runtime */
    static std::shared_ptr<const Type> get_type(Typename t)
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
            if ( !get_type(REFLECT_T) )
            {
                std::shared_ptr<Type> type = std::make_shared<meta_enum<REFLECT_T>>();
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
                std::shared_ptr<Type> type  = std::make_shared<Class>(_name);
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

    template<typename T>
    class meta_type<T*> : public meta_type<T> {};

    template<typename T>
    class meta_type<T&> : public meta_type<T> {};

}

