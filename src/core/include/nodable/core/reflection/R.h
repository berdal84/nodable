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

#include <memory> // std::shared_ptr

#include "nodable/core/types.h"
#include "nodable/core/assertions.h"
#include "R_Class.h"
#include "R_Meta_t.h"
#include "R_Qualifier.h"
#include "R_Type.h"
#include "R_MACROS.h"
#include "R_Register.h"

namespace Nodable { namespace R
{
    /**
     * set a value to an union member
     */
    template<typename U, typename T> void set_union(U& instance, T value)
    {
        T& member = U::template get<T>(instance);
        member = value;
    }

     /**
     * get a union member value
     */
     template<typename T, typename U> T& get_union(U& instance)
    {
        return U::template get<T>(instance);
    }

     /**
     * get a union member value
     */
     template<typename T, typename U> const T& get_union(const U& instance)
    {
        return U::template get<T>(instance);
    }

    /**
     * Given a Type::xxxx value, return its equivalent MetaType.
     */
    static Meta_t_csptr meta(Type type)
    {
        auto found = Register::by_type().find(type);
        if (found != Register::by_type().end() )
            return found->second;
        NODABLE_ASSERT(found->second != nullptr);
        return nullptr;
    };

    /**
     * Given a type T, return its equivalent MetaType.
     */
    template<typename T>
    static Meta_t_csptr meta()
    {
        Meta_t_csptr result = nullptr;

        /* In case T is a pointer or a reference, we reuse the meta_type without qualifier, and we add the desire
         * qualifier */
        if( std::is_pointer<T>::value )
        {
            Meta_t_csptr base = meta<typename std::remove_pointer<T>::type>();
            result = Meta_t::make_ptr(base );
        }
        else if( std::is_reference<T>::value )
        {
            Meta_t_csptr base = meta<typename std::remove_reference<T>::type>();
            result = Meta_t::make_ref(base );
        }
        else
        {
            std::map<std::string, Meta_t_csptr>& type_map = Register::by_typeid();
            auto found = type_map.find(typeid(T).name());
            if (found != type_map.end())
            {
                result = found->second;
            }
        }

        NODABLE_ASSERT(result != nullptr);
        return result;
    };

    template<class target_t, class source_t>
    static target_t* cast_class_ptr(source_t *_source)
    {
        static_assert(Register::has_class<source_t>(), "class Src is not reflected by R");
        static_assert(Register::has_class<target_t>(), "class Dst is not reflected by R" );

        if(_source->get_class()->is_child_of(target_t::Get_class()))
        {
            return dynamic_cast<target_t*>(_source);
        }
        return nullptr;
    };

    /** Simple struct to initialise R */
    struct Initialiser
    {
        Initialiser();
        void log_statistics();

    private:
        static bool s_initialized;
    };

} // namespace R
/** declare some correspondence between type and typeenum */
    R_DECLARE_LINK("double" , double      , Nodable::R::Type::double_t )
    R_DECLARE_LINK("int"    , i16_t       , Nodable::R::Type::i16_t )
    R_DECLARE_LINK("string" , std::string , Nodable::R::Type::string_t )
    R_DECLARE_LINK("bool"   , bool        , Nodable::R::Type::bool_t )
    R_DECLARE_LINK("void"   , void        , Nodable::R::Type::void_t )
} // namespace Nodable



