#pragma once

/**
 * The objective of these 3 struct is to:
 * - create a link between a typename and a TypeEnum,
 * - get the link with TypeEnum,
 * - get the link with a typename.
 */
#define R_DECLARE_LINK( T_STRING, T, TYPE ) \
    /** declare the link */  \
    template<> \
    struct Nodable::R::type_to_value<T, TYPE> \
    {  \
        using type = T; \
        static constexpr const char*    name       = T_STRING; \
        static constexpr decltype(TYPE) type_v     = TYPE; \
        static constexpr const char*    type_name  = #TYPE;\
        \
        static std::shared_ptr<Nodable::R::MetaType> new_meta_type() { \
            return std::make_shared<Nodable::R::MetaType>(T_STRING, TYPE); \
        }\
    }; \
    template<> struct \
    Nodable::R::reflect_value<TYPE> : Nodable::R::type_to_value<T, TYPE> {}; \
    template<> struct \
    Nodable::R::reflect_type<T> : Nodable::R::type_to_value<T, TYPE> {};

#define R_ENUM( Enum ) \
    static const char* to_string(Enum value) \
    {\
        using type = Enum; \
        switch( value )\
        {

#define R_ENUM_VALUE(VALUE) \
            case type::VALUE: return #VALUE;

#define R_ENUM_END \
            default: return "<not reflected>";\
        } \
    }
/**
 *  Set of MACROS to work with class Class (see Reflect_Class.h)
 */

/**
 * Must be inserted to start a reflection declaration, short version exist ex: R or R_WITH_INHERITANCE
 */
#define R_BEGIN( CLASS, ... ) \
public:\
    \
    virtual R::Class_ptr get_class() const __VA_ARGS__ { \
      return CLASS::Get_class(); \
    } \
    \
    static R::Class_ptr Get_class() { \
      static R::Class_ptr clss = CLASS::Reflect_class(); \
      return clss; \
    } \
    static R::Class_ptr Reflect_class() { \
      R::Class_ptr clss = std::make_shared<R::Class>(#CLASS);

/**
 * Must be inserted between R_BEGIN and R_END macro usage
 */
#define R_EXTENDS(PARENT_CLASS) \
      clss->add_parent( PARENT_CLASS::Get_class() ); \
      PARENT_CLASS::Get_class()->add_child( clss );

/**
 * Must be added after any usage of R_BEGIN, can be placed after a R_INHERITS
 */
#define R_END \
      return clss; /* return for CreateClass() */ \
    }

/*
 * Short-end to type a class with minimal information (ex: name)
 */
#define R( CLASS ) \
    R_BEGIN( CLASS ) \
    R_END

/**
 * Short-end to type a class with minimal information with inheritance information.
 */
#define R_DERIVED(CLASS ) \
    R_BEGIN( CLASS, override )

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
#define R_DEFINE_CLASS( CLASS ) \
static auto reflected_##_CLASS = Nodable::R::Register::push<CLASS>();

