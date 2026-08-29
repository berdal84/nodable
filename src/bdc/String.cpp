#include "String.hpp"

namespace bdc
{
    String& String::operator=(const String& data)
    {
        if ( this == &data ) return *this;
        memcpy(static_cast<void*>(this), static_cast<const void*>(&data), sizeof(String));
        return *this;
    }

    const i8_t* String::c_str() const
    {
        return string_cstr(*this);
    }

    String string_concat(const String& a, const String& b, Allocator* allocator )
    {
        //printf( "a: '%s' (size: %i)\n", a.c_str(), a.size );
        //printf( "b: '%s' (size: %i)\n", b.c_str(), b.size );

        u32_t alloc_size = a.size + b.size + 1 ; // I am unsure this is a good idea, but I prefer to allocate 1 byte extra for null-termination
        i8_t* alloc_data = memory_malloc_array<i8_t>( alloc_size ); 

        String result;
        result.data = alloc_data;
        result.size = alloc_size - 1;          

        //printf( "result: '%s'\n", result.c_str());
        memcpy(result.data          , a.data, a.size ); //printf( "result: '%s'\n", result.c_str());
        memcpy(result.data + a.size , b.data, b.size);  //printf( "result: '%s'\n", result.c_str());

        alloc_data[alloc_size-1] = '\0';

        return result;
    }

    void string_reset(String& str)
    {
        str = {};
    }

    void string_release(String& str, Allocator* release_allocator )
    {
        release_allocator->proc_free(str.data);
        string_reset(str);
    }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    u32_t string_rfind(const String& str, i8_t c)
    {
        u32_t cursor = str.size-1;
        while ( cursor != String::String::invalid_pos && str.data[cursor] != c)
        {
            --cursor;
        }

        return cursor;
    }

    String string_lsplit(const String& str, u32_t index)
    {
        assert(index <= str.size && "Out of bounds");

        if( index == str.size)
        {
            return str;
        }

        String result = str;
        result.size -= result.size - index;
        result.flags &= ~String_Flags_IS_NULL_TERMINATED; // remove flag, we cut in the middle

        return result;
    }

    String string_rsplit(const String& str, u32_t index)
    {
        assert(index < str.size && "Out of bounds");

        if( index == 0)
        {
            return str;
        }

        String result = str;
        result.data += index;
        result.size -= index;
        result.flags |= str.flags & String_Flags_IS_NULL_TERMINATED;

        return result;
    }

    String string_basename(const String& str)
    {
        u32_t last_slash = string_rfind(str, '\\');
        if ( last_slash == String::invalid_pos )
        {
            return str;
        }
        return string_rsplit(str, last_slash+1);
    }

    String string_stem(const String& str)
    {
        u32_t index = string_rfind(str, '.');
        if( index == String::invalid_pos )
        {
            return str;
        }
        return string_lsplit(str, index);
    }

    const i8_t* string_cstr(const String& str)
    {
        if ( (str.flags & String_Flags_IS_NULL_TERMINATED) || str.data == nullptr )
        {
            return str.data;
        }

        String result = string_printf(temp_allocator(), "%.*s", str.size, str.data);

        return result.data;
    }

    String string_copy(const String& source, Allocator* copy_allocator )
    {
        if( source.size == 0)
            return {};
            
        String result{};
        string_copy( result, source, copy_allocator);
        return result;
    }


    String& string_copy(String& target, const String& source, Allocator* copy_allocator )
    {
        target.data = memory_malloc_array<i8_t>(source.size, copy_allocator);
        target.size = source.size;

        std::memcpy(target.data, source.data, source.size + 1); // null terminated

        target.data[source.size] = '\0';

        return target;
    }

    int string_compare(const String& a, const String& b)
    {
        return a.size == b.size && strncmp(a.data, b.data, a.size) == 0;
    }

    String string_case_insensitive_find(const String& haystack, const String& needle)
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

    String  string_unquote(const String& str)
    {
        assert(str.size >= 2);
        assert(str[0] == '\"');
        assert(str[str.size-1]  == '\"');

        return String{ str.data + 1, str.size -2};
    }

    String string_view(const String& str)
    {
        return String(str.data, str.size);
    };

    String& string_advance(String& str, u32_t amount)
    {
        assert(str.size >= amount && "String is too short to advance that amount");
        str.data += amount;
        str.size -= amount;

        return str;
    }

    bool operator<(const String& a, const String& b)
    {
        // Compare the
        const int n = strncmp(a.data, b.data, a.size > b.size ? a.size : b.size );

        return n < 0
          || ( n == 0 && a.size < b.size);
    }

    bool operator==(const String& a, const String& b)
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
    
    bool operator!=(const String& a, const String& b)
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
