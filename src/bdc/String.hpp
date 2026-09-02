#pragma once
#include "Allocators.hpp"
#include "Array.hpp"
#include <cassert>
#include <cstring>
#include <ctype.h> // for toupper / tolower
#include "Types.hpp"
#include "Type_Traits.hpp"

namespace bdc 
{
    typedef u8_t String_Flags;
    enum String_Flags_
    {
        String_Flags_NONE                 = 0,
        String_Flags_IS_NULL_TERMINATED   = 1 << 0,

        // Note: user may use the remaining bits to store whatever.
    };
    
    //
    // String is super inspired by Jai language's strings. However, it is not a builtin type in C++ of course.
    // The main goal of it is to serves as replacement for const char* and avoid calling strlen frequently.
    // The main difference with const char* - besides having a size - is that the data may not be null-terminated.
    // That offers some nice features like free-allocation slicing.
    // String can be converted to const char* at any time, by using String::s_str() or string_to_cstr(String&).
    //
    // - ..is a view,
    // - ..has flags to know if the pointed buffer has null terminator,
    // - ..is limited to almost 32-bits long strings (u32_t-1 is reserved),
    //
    struct String
    {
        static constexpr u32_t invalid_pos = (u32_t)-1; // depends on context
        
        u32_t           size = 0; // string size ALWAYS excludes the null terminator, allocated buffer might be larger IF AND ONLY IF flags has String_Flags_IS_NULL_TERMINATED
                              // size must be 1st, because it has to match with Array, Resizable_Array, and Inlined_Array memory layout.
                              // it costs memory (8 bytes) but it is convenient when threating String as an Array<i8_t>.
        i8_t*           data = nullptr; // may be a valid const char* IF AND ONLY IF flags has String_Flags_IS_NULL_TERMINATED
        String_Flags    flags = 0;

        // note: constexpr constructor must be defined in the header (just below string_xxx API)
        constexpr       String() = default;
        constexpr       String(const i8_t* cstr);
        template<size_t N>
        constexpr       String(const i8_t (&cstr)[N] );
        constexpr       String(i8_t* data, u32_t len, String_Flags _flags = 0);
        constexpr       String(const i8_t& c);
        constexpr       String(const Array<i8_t>& arr);
        constexpr       String(const String& other);
        
        constexpr       ~String() = default;
 
        inline i8_t     operator[](u32_t pos) const { assert(pos < size && "String_View::operator[]() const - out of bounds position!"); return data[pos]; };
        inline i8_t&    operator[](u32_t pos) { assert(pos < size && "String_View::operator[]() - out of bounds position!"); return data[pos]; };       
        inline          operator Array<i8_t>& () { return *reinterpret_cast<Array<i8_t>*>(this); }
        inline          operator const Array<i8_t>& () const { return *reinterpret_cast<const Array<i8_t>*>(this); }
        String&         operator=(const String&);
        
        const i8_t*     c_str() const;
        inline bool     empty() const { return size == 0; }
    };
    static_assert( sizeof(String) == 24, "String has an unexpected size!" );

    void            string_reset(String&);
    void            string_release(String&, Allocator* release_allocator = default_allocator());
    u32_t           string_rfind(const String&, i8_t c);
    String          string_lsplit(const String&, u32_t index);
    String          string_rsplit(const String&, u32_t index);
    String          string_basename(const String&);
    String          string_stem(const String&);
    const i8_t*     string_cstr(const String&);
    String          string_copy(const String& source, Allocator* copy_allocator = default_allocator() );
    String&         string_copy(String& target, const String& source, Allocator* copy_allocator = default_allocator() );
    int             string_compare(const String&, const String&);
    String          string_printf(Allocator* allocator, const i8_t* fmt, auto&&...args);
    String          string_printf(const i8_t* fmt, auto&&...args );
    String          string_concat(const String& a, const String& b, Allocator* = default_allocator() );
    String          string_case_insensitive_find(const String& haystack, const String& needle);
    String          string_unquote(const String&);
    String          string_view(const String&);
    String&         string_advance(String&, u32_t amount);
    bool            operator<(const String& a, const String& b);
    bool            operator==(const String& a, const String& b);
    bool            operator!=(const String& a, const String& b);

    // note: constexpr stuff must be declared in the header

    constexpr u32_t constexpr_strlen(const i8_t* cstr)
    {
        size_t len = 0;
        while( cstr[len] != '\0')
        {
            ++len;
        }
        assert(len < String::invalid_pos);
        return (u32_t)len;
    }

    template<size_t N> // N = strlen(cstr) + 1
    constexpr String::String(const i8_t (&cstr)[N] )
    : data(const_cast<i8_t*>(cstr))
    , size(N)
    , flags(String_Flags_IS_NULL_TERMINATED)
    {
    }

    constexpr String::String(i8_t* data, u32_t len, String_Flags _flags)
    : data(const_cast<i8_t*>(data))
    , size(len)
    , flags(_flags)
    {}

    constexpr String::String(const i8_t& c)
    : data(const_cast<i8_t*>(&c))
    , size(1)
    , flags(0)
    {
    }

    constexpr String::String(const i8_t* cstr)
    : data(const_cast<i8_t*>(cstr))
    , size(constexpr_strlen(cstr))
    , flags(String_Flags_IS_NULL_TERMINATED)
    {}

    constexpr String::String(const Array<i8_t>& arr)
    : data(const_cast<i8_t*>(arr.data))
    , size(arr.size)
    {}

    constexpr String::String(const String& other)
    : data(other.data)
    , size(other.size)
    , flags(other.flags)
    {}

    String string_printf(const i8_t* fmt, auto&&...args )
    {
        return string_printf(temp_allocator(), fmt, std::forward<decltype(args)>(args)...);
    }

    String string_printf(Allocator* allocator, const i8_t* fmt, auto&&...args )
    {   
        static_assert( sizeof...(args) != 0, "No arguments, use string_copy instead.");

        // compute required size
        size_t required_alloc_size = snprintf( nullptr, 0, fmt, std::forward<decltype(args)>(args)... ) + 1; // +1 for null terminator
        
        if (required_alloc_size < 0) 
        {
            // Handle encoding error
            return {};
        }

        // allocate
        assert(required_alloc_size < String::invalid_pos);
        i8_t* string_data = memory_malloc_array<i8_t>(required_alloc_size, allocator);
        assert(string_data && "Unable to allocate memory for string_printf!");

        // print
        u32_t string_len = (u32_t)required_alloc_size;
        snprintf( string_data, string_len, fmt, std::forward<decltype(args)>(args)...);

        return String{ string_data, string_len-1, String_Flags_IS_NULL_TERMINATED };
    }
    
} // namespace bdc