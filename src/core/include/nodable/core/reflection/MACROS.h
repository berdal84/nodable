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

#define REFLECT_ENABLE() \
public: type get_type() const; \
private:


