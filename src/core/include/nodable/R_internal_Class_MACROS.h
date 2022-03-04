#pragma once

/**
 *  Set of MACROS to work with class Class (see Reflect_Class.h)
 */

/**
 * Must be inserted to start a reflection declaration, short version exist ex: R or R_WITH_INHERITANCE
 */
#define R_BEGIN( _CLASS, ... ) \
public:\
    \
    virtual R::Class* get_class() const __VA_ARGS__ { \
      return _CLASS::Get_class();\
    } \
    \
    static R::Class* Get_class() {  \
      static R::Class* clss = _CLASS::Reflect_class(); \
      return clss; \
    } \
    \
    static R::Class* Reflect_class() {   \
      R::Class* clss = new R::Class(#_CLASS);

/**
 * Must be inserted between R_BEGIN and R_END macro usage
 */
#define R_EXTENDS(_PARENT_CLASS) \
      clss->add_parent( _PARENT_CLASS::Get_class() ); \
      _PARENT_CLASS::Get_class()->add_child( clss );

/**
 * Must be added after any usage of R_BEGIN, can be placed after a R_INHERITS
 */
#define R_END \
      return clss; /* return for CreateClass() */ \
    }

/*
 * Short-end to type a class with minimal information (ex: name)
 */
#define R( _CLASS ) \
    R_BEGIN( _CLASS ) \
    R_END

/**
 * Short-end to type a class with minimal information with inheritance information.
 */
#define R_DERIVED(_CLASS ) \
    R_BEGIN( _CLASS, override )

/**
 * Must be added to your class *.cpp file in order to generate MetaClass before main() starts.
 * note this is not always required, for example with
 *
 * class A { ... };
 * class B : class A { ... }
 *
 * here A DEFINITION can be omitted since B DEFINITION will also define A.
 *
 * It works with N level(s) of inheritance too.
 * A <- B <- C <- D, here only D needs to be explicitly defined in it's cpp.
 *
 */
#define R_DEFINE_CLASS( _CLASS ) \
    static Nodable::R::register_class<_CLASS> MAKE_UNIQUE_VAR_NAME(inserter) (#_CLASS) ; \
    static_assert( Nodable::R::is_class_reflected<_CLASS>::value, "class "#_CLASS" is not reflected by R" );
