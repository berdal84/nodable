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
#include "R_MetaType.h"
#include "R_Qualifier.h"
#include "R_Type.h"
#include "R_MACROS.h"
#include "R_Register.h"

namespace Nodable { namespace R
{

    /**
     * Initialize R.
     * - register base types
     * - log statistics
     */
    void init();

    void log_statistics();

    /**
     * Given a Type::xxxx value, return its equivalent MetaType.
     */
    static std::shared_ptr<const MetaType> get_meta_type(Type t)
    {
        auto found = Register::by_type().find(t);
        if (found != Register::by_type().end() )
            return found->second;
        NODABLE_ASSERT(found->second != nullptr);
        return nullptr;
    };

    /**
     * Given a type T, return its equivalent MetaType.
     */
    template<typename T>
    static std::shared_ptr<const MetaType> get_meta_type()
    {
        std::shared_ptr<const MetaType> result = nullptr;

        /* In case T is a pointer or a reference, we reuse the meta_type without qualifier, and we add the desire
         * qualifier */
        if( std::is_pointer<T>::value )
        {
            std::shared_ptr<const MetaType> base = get_meta_type<typename std::remove_pointer<T>::type>();
            result = MetaType::make_ptr(base );
        }
        else if( std::is_reference<T>::value )
        {
            std::shared_ptr<const MetaType> base = get_meta_type<typename std::remove_reference<T>::type>();
            result = MetaType::make_ref(base );
        }
        else
        {
            auto found = Register::by_typeid().find(typeid(T).name());
            if (found != Register::by_typeid().end())
            {
                result = found->second;
            }
        }

        NODABLE_ASSERT(result != nullptr);
        return result;
    };

    template<class Dst, class Src>
    static Dst* cast_pointer(Src *_source)
    {
        static_assert(Register::has_class<Src>(), "class Src is not reflected by R");
        static_assert(Register::has_class<Dst>(), "class Dst is not reflected by R" );

        if(_source->get_class()->is_child_of(Dst::Get_class()))
            return dynamic_cast<Dst*>(_source);
        return nullptr;
    };
} // namespace R
/** declare some correspondence between type and typeenum */
    R_DECLARE_LINK("double" , double      , Nodable::R::Type::double_t )
    R_DECLARE_LINK("int"    , i16_t       , Nodable::R::Type::i16_t )
    R_DECLARE_LINK("string" , std::string , Nodable::R::Type::string_t )
    R_DECLARE_LINK("bool"   , bool        , Nodable::R::Type::bool_t )
    R_DECLARE_LINK("void"   , void        , Nodable::R::Type::void_t )
} // namespace Nodable



