#pragma once

/***********************************************************************************************************************
 *  UNIONS
 */

/**
 * call this macro from the end of your union to reflect it
 */
#define R_UNION(U) /* U: Union, M: Union member */      \
    template<typename T> void set(T _value);            \
    template<typename T> T& get();                      \
    template<typename T> const T& get() const;

/**
* call this once per member in your *.cpp file or out of the union
*/
#define R_UNION_MEMBER_DEFINITION(U, M) /* U: Union, M: Union member */  \
    template<>\
    void U::set<decltype(U::M)>(decltype(U::M) _value)\
    {\
         M = _value;\
    }\
    template<>\
    decltype(U::M)& U::get<decltype(U::M)>()\
    {\
        return M;\
    }\
    template<>\
    const decltype(U::M)& U::get<const decltype(U::M)>()\
    {\
        return M;\
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

#define REFLECT_BASE_CLASS() \
public:\
    virtual type get_type() const { return type::get<decltype(this)>(); }\
private:

#define REFLECT_DERIVED_CLASS(...) \
public:\
    virtual type get_type() const override { return type::get<decltype(this)>(); }\
private:

#define CAT_IMPL(a, b) a##b
#define CAT(a, b) CAT_IMPL(a, b)

#define REGISTER                                                        \
static void auto_register();                                            \
namespace /* using the same trick as rttr to avoid name conflicts*/     \
{                                                                       \
    struct auto_register_struct                                         \
    {                                                                   \
        auto_register_struct()                                          \
        {                                                               \
            auto_register();                                            \
        }                                                               \
    };                                                                  \
}                                                                       \
static const auto_register_struct CAT(auto_register, __LINE__);         \
static void auto_register()

