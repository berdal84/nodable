#pragma once

/***********************************************************************************************************************
 *  UNIONS
 */

/**
 * call this macro from the end of your union to reflect it
 */
#define R_UNION(U) /* U: Union, M: Union member */ \
    template<typename T> static const T& get(const U& _instance); \
    template<typename T> static T& get(U& _instance);

/**
* call this once per member in your *.cpp file or out of the union
*/
#define R_UNION_MEMBER_DEFINITION(U, M) /* U: Union, M: Union member */  \
    template<>\
    decltype(U::M)& U::get<decltype(U::M)>(U& instance)\
    {\
        return instance.M;\
    }\
    template<>\
    const decltype(U::M)& U::get<const decltype(U::M)>(const U& instance)\
    {\
        return instance.M;\
    }

/***********************************************************************************************************************
*  TYPE <---> Type enum
*/

/**
 * The objective of these 3 struct is to:
 * - create a link between a typename and a TypeEnum,
 * - get the link with TypeEnum,
 * - get the link with a typename.
 */
#define R_REGISTER( TYPE_AS_STRING, TYPE ) \
    template<> struct \
    Nodable::Registration::push<TYPE>{};

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

/***********************************************************************************************************************
*  CLASSES
*/

#define R_CLASS_BEGIN( CLASS, ... ) \
public:\
    \
    virtual type get_type() const __VA_ARGS__ { \
      return type::get<CLASS>(); \
    } \
    static type Reflect_class() { \
      type type(typeid(CLASS).hash_code(), #CLASS);

/**
 * Must be inserted between R_BEGIN and R_CLASS_END macro usage
 */
#define R_CLASS_EXTENDS(PARENT_CLASS) \
      type.add_parent( type::get<PARENT_CLASS>() ); \
      type::get<PARENT_CLASS>().add_child( type );

/**
 * Must be added after any usage of R_BEGIN, can be placed after a R_INHERITS
 */
#define R_CLASS_END \
      return type; /* return for CreateClass() */ \
    }

/*
 * Short-end to type a class with minimal information (ex: name)
 */
#define R_CLASS( CLASS ) \
    R_CLASS_BEGIN( CLASS ) \
    R_CLASS_END

/**
 * Short-end to type a class with minimal information with inheritance information.
 */
#define R_CLASS_DERIVED(CLASS ) \
    R_CLASS_BEGIN( CLASS, override )

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
static auto reflected_##_CLASS = Nodable::registration::push<CLASS>(#CLASS);

