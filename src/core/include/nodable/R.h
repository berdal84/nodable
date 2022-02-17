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
/*
 * Include Reflect dependencies
 */
#include <nodable/R_Class.h>
#include <nodable/R_Class_MACROS.h>
#include <nodable/R_Type.h>
#include <nodable/R_Type_MACROS.h>

#define ENABLE_ANY_PTR_TO_BE_CONSIDERED_VOID_PTR true

namespace Nodable::R
{
    static void Initialize()
    {
        // placeholder
    }

    /*
     * Define some basic types...
     */

    using unknown_t = std::nullptr_t; // <----- seriously doubting about this idea.

    R_DEF_TYPE(unknown_t    , Type::Unknown )
    R_DEF_TYPE(double       , Type::Double )
    R_DEF_TYPE(double&      , add_ref(Type::Double) )
    R_DEF_TYPE(double*      , add_ptr(Type::Double) )
    R_DEF_TYPE(std::string  , Type::String )
    R_DEF_TYPE(std::string& , add_ref(Type::String )  )
    R_DEF_TYPE(std::string* , add_ptr(Type::String ) )
    R_DEF_TYPE(bool         , Type::Boolean )
    R_DEF_TYPE(bool&        , add_ref(Type::Boolean ) )
    R_DEF_TYPE(bool*        , add_ptr(Type::Boolean ) )
    R_DEF_TYPE(void*        , add_ptr(Type::Void) )

#if ENABLE_ANY_PTR_TO_BE_CONSIDERED_VOID_PTR
/*
 * To avoid the ned to overdeclare, this templae allows any object pointer to be treater as a void*
 * ex: Node*, Object*, Car*, Character* will be treated as void*
 */
    template<typename T>
    struct cpp<T*>
    {
        using meta = cpp<void*>::meta;
    };
#endif
}

