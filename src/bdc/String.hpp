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
    //
    // String:
    // - ..is super inspired by Jai's strings.
    // - ..is a view,
    // - ..is limited to almost 32-bits long strings (u32_t-1 is reserved),
    //
    struct String
    {
        static_assert(std::is_same_v<i8_t, char>);

        static constexpr u32_t size_max    = (u32_t)-1; // largest uint is reserved...
        static constexpr u32_t invalid_pos = size_max;  // ...meaning depends on context.

        // string size ALWAYS excludes the null terminator, allocated buffer might be larger IF AND ONLY IF flags has String_Flags_IS_NULL_TERMINATED
        // size must be 1st, because it has to match with Array, Resizable_Array, and Inlined_Array memory layout.
        // it costs memory (8 bytes) but it is convenient when threating String as an Array<i8_t>.
        u32_t           size  = 0;
        i8_t*           data  = nullptr; // may be a valid null-terminated const char* IF AND ONLY IF flags has String_Flags_IS_NULL_TERMINATED

        inline i8_t     operator[](u32_t pos) const;
        inline i8_t&    operator[](u32_t pos);       
        inline          operator Array<i8_t>& ();
        inline          operator const Array<i8_t>& () const;
    };
    static_assert( std::is_trivially_copyable_v<String> );
    static_assert( sizeof(String) <= 24, "String has an unexpected size!" );

    //
    // String API
    //

    // Note: there is no string(i8_t*, u32_t, String_Flags) procedure, you have to use the struct String itself.
    //       String s{ .size = ..., .data = ..., .flags = ... };

    constexpr String    string(const i8_t& single_character);
    constexpr String    string(const std::string_view&);     
    constexpr String    string_from_c_str(const i8_t* null_terminated_str);
    constexpr String    string(const Array<i8_t>& arr);    
    inline String       new_string(u32_t size, bool zero_init = true);
    inline const i8_t*  c_str(const String& str);

    inline String       string_tprintf(const i8_t* fmt, auto&&...args );
    inline String       string_printf(const i8_t* fmt, auto&&...args );
    inline String       string_concat(const String& a, const String& b );
    inline void         string_reset(String& str);
    inline void         string_release(String& str );
    inline u32_t        string_rfind(const String& str, i8_t character);
    inline String       string_lsplit(const String& str, u32_t index);
    inline String       string_rsplit(const String& str, u32_t index);
    inline String       string_basename(const String& str);
    inline String       string_stem(const String& str);
    inline String       string_tcopy(const String& source);
    inline String       string_copy(const String& source );
    inline int          string_compare(const String& a, const String& b);
    inline String       string_case_insensitive_find(const String& haystack, const String& needle);
    inline String       string_unquote(const String& str);
    inline String&      string_advance(String& str, u32_t amount);

    inline bool         operator<(const String& a, const String& b);
    inline bool         operator==(const String& a, const String& b);    
    inline bool         operator!=(const String& a, const String& b);

    // constexpr version of strlen
    constexpr u32_t _constexpr_strlen(const i8_t* cstr);

    //
    // Implementation:
    //

    inline String new_string(u32_t size, bool zero_init)
    {
        BDC_ASSERT(size < String::size_max);

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

    constexpr u32_t _constexpr_strlen(const i8_t* cstr)
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
        BDC_ASSERT(len < String::size_max);
        return (u32_t)len;
    }

    template<size_t N> // N = strlen(cstr) + 1
    constexpr String string(const char (&literal_c_str)[N] )
    {
        static_assert( std::is_same_v<char, i8_t> );

        return {
            .size  = N-1,
            .data  = const_cast<i8_t*>(literal_c_str),
        };
    }

    constexpr String string(const i8_t* null_terminated_str)
    {
        return {
            .size  = _constexpr_strlen(null_terminated_str),
            .data  = const_cast<i8_t*>(null_terminated_str),
        };
    }

    constexpr String string(const i8_t& single_char)
    {
        return {
            .size  = 1,
            .data  = const_cast<i8_t*>(&single_char),
        };
    }
    
    constexpr String string(const std::string_view& s)
    {
        BDC_ASSERT( s.size() < String::size_max);
        return {
            .size = (u32_t)s.size(),
            .data = const_cast<i8_t*>(s.data()),
        };
    }

    constexpr String string(const Array<i8_t>& arr)
    {
        return {
            .size = arr.size,
            .data = const_cast<i8_t*>(arr.data)
        };
    }

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
        BDC_ASSERT(required_alloc_size < String::size_max);
        String result = new_string((u32_t)required_alloc_size);

        // print
        snprintf( result.data, result.size, fmt, std::forward<decltype(args)>(args)...);

        // hide the '\0' char
        result.size  -= 1;

        return result;
    }

    inline String string_concat(const String& a, const String& b )
    {
        String result = new_string(a.size + b.size);

        memcpy(result.data          , a.data, a.size );
        memcpy(result.data + a.size , b.data, b.size);

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

        return result;
    }

    inline String string_rsplit(const String& str, u32_t index)
    {
        BDC_ASSERT(index <= str.size, "Out of bounds");

        if( index == 0 )
        {
            return String{
                .size = 0,
                .data = str.data,
            };
        }


        String result = str;
        result.data += index;
        result.size -= index;

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

    inline const i8_t* c_str(const String& str)
    {
        if ( str.data == nullptr )
        {
            return "";
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
                return String{
                    .size = needle.size,
                    .data = haystack.data + i
                };
            }
        }
        return {};
    }

    inline String string_unquote(const String& str)
    {
        BDC_ASSERT(str.size >= 2);
        BDC_ASSERT(str[0] == '\"');
        BDC_ASSERT(str[str.size-1]  == '\"');

        return String{
            .size = str.size - 2,
            .data = str.data + 1,
        };
    }

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

    inline i8_t     String::operator[](u32_t pos) const
    {
        BDC_ASSERT(pos < size, "pos is out of bounds"); return data[pos];
    };

    inline i8_t&    String::operator[](u32_t pos)
    {
        BDC_ASSERT(pos < size, "pos is out of bounds");
        return data[pos];
    };       

    inline String::operator Array<i8_t>& ()
    {
        return *reinterpret_cast<Array<i8_t>*>(this);
    }

    inline String::operator const Array<i8_t>& () const
    {
        return *reinterpret_cast<const Array<i8_t>*>(this);
    }
    
} // namespace bdc