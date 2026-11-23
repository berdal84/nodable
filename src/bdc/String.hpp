#pragma once

#include <cassert>
#include <cstring>
#include <ctype.h>  // for toupper / tolower
#include <cstdio>   // for printf & co.

#include "Allocators.hpp"
#include "Array.hpp"
#include "Types.hpp"
#include "Type_Traits.hpp"

// to use with printf
// ex: printf( "Value: " STRING_FMT, STRING_VA( str ) );
//
#define STRING_FMT "%.*s"
#define STRING_FMT_ARGS( str ) str.size, str.data

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
    // String is super inspired by Jai parser's strings. However, it is not a builtin type in C++ of course.
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
 
        inline i8_t     operator[](u32_t pos) const          { BDC_ASSERT(pos < size, "pos is out of bounds"); return data[pos]; };
        inline i8_t&    operator[](u32_t pos)                { BDC_ASSERT(pos < size, "pos is out of bounds"); return data[pos]; };       
        inline          operator Array<i8_t>& ()             { return *reinterpret_cast<Array<i8_t>*>(this); }
        inline          operator const Array<i8_t>& () const { return *reinterpret_cast<const Array<i8_t>*>(this); }
        String&         operator=(const String&) = default;
        
        const i8_t*     c_str() const;
        inline bool     empty() const { return size == 0; }
    };
    static_assert( sizeof(String) <= 24, "String has an unexpected size!" );

    void            string_reset(String&);
    void            string_release(String&);
    u32_t           string_rfind(const String&, i8_t c);
    String          string_lsplit(const String&, u32_t index);
    String          string_rsplit(const String&, u32_t index);
    String          string_basename(const String&);
    String          string_stem(const String&);
    const i8_t*     string_cstr(const String&);
    String          string_copy(const String& source);
    String          string_tcopy(const String& source);
    String&         string_copy(String& target, const String& source);
    int             string_compare(const String&, const String&);
    String          string_tprintf(const char* fmt, auto&& ...args);
    String          string_printf(const char* fmt, auto&&...args );
    String          string_concat(const String& a, const String& b);
    String          string_case_insensitive_find(const String& haystack, const String& needle);
    String          string_unquote(const String&);
    String          string_view(const String&);
    String&         string_advance(String&, u32_t amount);
    bool            operator<(const String& a, const String& b);
    bool            operator==(const String& a, const String& b);
    bool            operator!=(const String& a, const String& b);

    inline String new_string(u32_t size, bool zero_init = true)
    {
        String result{};

        result.data = memory_malloc_array<i8_t>( size );
        BDC_ASSERT(result.data, "Unable to allocate memory for result!");
        result.size = size;

        if( zero_init )
        {
            memset(result.data, 0, result.size);
        }

        return result;
    }

    // note: constexpr stuff must be declared in the header too

    constexpr u32_t constexpr_strlen(const i8_t* cstr)
    {
        if( cstr == nullptr)
        {
            return 0;
        }
        
        size_t len = 0;
        while( cstr[len] != '\0')
        {
            ++len;
        }
        BDC_ASSERT(len < String::invalid_pos, "cstr is too long for String's buffer");
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

    inline String string_tprintf(const i8_t* fmt, auto&&...args )
    {
        push_allocator( &temp_allocator );
        String result = string_printf(fmt, std::forward<decltype(args)>(args)...);
        pop_allocator();
        return result;
    }

    inline String string_printf(const i8_t* fmt, auto&&...args )
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
        BDC_ASSERT(required_alloc_size < String::invalid_pos);
        String result = new_string((u32_t)required_alloc_size);

        // print
        snprintf( result.data, result.size, fmt, std::forward<decltype(args)>(args)...);

        // hide the '\0' char
        result.size  -= 1;

        // make the string aware of it
        result.flags |= String_Flags_IS_NULL_TERMINATED;

        return result;
    }

    inline const i8_t* String::c_str() const
    {
        return string_cstr(*this);
    }

    inline String string_concat(const String& a, const String& b )
    {
        String result = new_string(a.size + b.size + 1); // +1 for null termination

        memcpy(result.data          , a.data, a.size );
        memcpy(result.data + a.size , b.data, b.size);

        result[result.size-1] = '\0';
        result.size  -= 1;
        result.flags |= String_Flags_IS_NULL_TERMINATED;

        return result;
    }

    inline void string_reset(String& str)
    {
        str.data = nullptr;
        str.size = 0;
    }

    inline void string_release(String& str )
    {
        allocator->proc_free(str.data);
        string_reset(str);
    }

    inline u32_t string_rfind(const String& str, i8_t c)
    {
        u32_t cursor = str.size-1;
        while ( cursor != String::String::invalid_pos && str.data[cursor] != c)
        {
            --cursor;
        }

        return cursor;
    }

    inline String string_lsplit(const String& str, u32_t index)
    {
        BDC_ASSERT(index <= str.size, "Out of bounds");

        if( index == str.size)
        {
            return str;
        }

        String result = str;
        result.size -= result.size - index;
        result.flags &= ~String_Flags_IS_NULL_TERMINATED; // remove flag, we cut in the middle

        return result;
    }

    inline String string_rsplit(const String& str, u32_t index)
    {
        BDC_ASSERT(index <= str.size, "Out of bounds");

        if( index == 0 )
        {
            return { str.data, 0 };
        }


        String result = str;
        result.data += index;
        result.size -= index;
        result.flags |= str.flags & String_Flags_IS_NULL_TERMINATED;

        return result;
    }

    inline String string_basename(const String& str)
    {
        u32_t last_slash = string_rfind(str, '\\');
        if ( last_slash == String::invalid_pos )
        {
            return str;
        }
        return string_rsplit(str, last_slash+1);
    }

    inline String string_stem(const String& str)
    {
        u32_t index = string_rfind(str, '.');
        if( index == String::invalid_pos )
        {
            return str;
        }
        return string_lsplit(str, index);
    }

    inline const i8_t* string_cstr(const String& str)
    {
        if ( str.data == nullptr )
        {
            return "";
        }
        
        if ( (str.flags & String_Flags_IS_NULL_TERMINATED))
        {
            return str.data;
        }

        String result = string_tprintf("%.*s", str.size, str.data);

        return result.data;
    }

    inline String string_tcopy(const String& source)
    {
        push_allocator(&temp_allocator);
        String result = string_copy(source);
        pop_allocator();
        return result;
    }

    inline String string_copy(const String& source )
    {
        String result{};

        result.data = memory_malloc_array<i8_t>(source.size + 1, allocator); // +1 for null terminator
        result.size = source.size;

        std::memcpy(result.data, source.data, source.size); 

        result.data[source.size] = '\0';

        return result;
    }

    inline int string_compare(const String& a, const String& b)
    {
        const u32_t size_min = a.size > b.size ? b.size : a.size;
         
        int n = strncmp(a.data, b.data, size_min);
        if ( n == 0 )
        {
            return  a.size < b.size ? 1 : -1;
        }
        return n;
    }

    inline String string_case_insensitive_find(const String& haystack, const String& needle)
    {
        if ( needle.size == 0 || needle.data == nullptr)
        {
            return {};
        }
        
        if ( needle.size > haystack.size )
        {
            return {};
        }
        
        for (u32_t i = 0; i <= haystack.size - needle.size; i++)
        {
            bool match = true;

            for (u32_t j = 0; j < needle.size; j++)
            {
                if ( tolower(haystack[i + j]) != tolower(needle[j]) )
                {
                    match = false;
                    break;
                }
            }

            if (match)
            {
                return String{ haystack.data + i, needle.size };
            }
        }
        return {};
    }

    inline String string_unquote(const String& str)
    {
        BDC_ASSERT(str.size >= 2);
        BDC_ASSERT(str[0] == '\"');
        BDC_ASSERT(str[str.size-1]  == '\"');

        return String{ str.data + 1, str.size -2};
    }

    inline String string_view(const String& str)
    {
        return String(str.data, str.size);
    };

    inline String& string_advance(String& str, u32_t amount)
    {
        BDC_ASSERT(str.size >= amount, "String is too short to advance that amount");
        str.data += amount;
        str.size -= amount;

        return str;
    }

    inline bool operator<(const String& a, const String& b)
    {
        // Compare the
        const int n = strncmp(a.data, b.data, a.size > b.size ? a.size : b.size );

        return n < 0
          || ( n == 0 && a.size < b.size);
    }

    inline bool operator==(const String& a, const String& b)
    {
        if( a.size != b.size)
        {
            return false;
        }

        u32_t cursor = 0;
        while( cursor < a.size )
        {
            if( a[cursor] != b[cursor] )
            {
                return false;
            }
            ++cursor;
        }

        return true;
    }
    
    inline bool operator!=(const String& a, const String& b)
    {
        if( a.size != b.size)
        {
            return true;
        }

        u32_t cursor = 0;
        while( cursor < a.size )
        {
            if( a[cursor] != b[cursor] )
            {
                return true;
            }
            ++cursor;
        }

        return false;
    }
    
} // namespace bdc