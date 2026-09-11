#pragma once

/**
 * call this macro from the end of your union to reflect it
 */
#define R_UNION(U) /* U: Union, M: Union property */      \
    template<typename T> void set(T _value);            \
    template<typename T> T& get();                      \
    template<typename T> const T& get() const;

/**
* call this once per get_value in your *.cpp file or out of the union
*/
#define R_UNION_MEMBER_DEFINITION(U, M) /* U: Union, M: Union property */  \
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
    const decltype(U::M)& U::get<const decltype(U::M)>() const\
    {\
        return M;\
    }
