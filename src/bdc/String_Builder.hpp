#pragma once
#include "String.hpp"
#include "Allocators.hpp"
#include "Types.hpp"
#include "Type_Traits.hpp"

namespace bdc
{
    struct String_Builder
    {
        Resizable_Array<String> buffer = { &temp_allocator };
    };

    void            string_builder_release(String_Builder&);
    void            string_builder_reset(String_Builder&);
    String_Builder& string_builder_append(String_Builder&, const String& str);
    String_Builder& string_builder_append(String_Builder&, const Array<String>& arr);
    String_Builder& string_builder_appendf(String_Builder& sb, const char* fmt, auto...args);
    String          string_builder_build_string(String_Builder& sb, String separator = "");
    String          string_builder_build_tstring(String_Builder&, String separator = "");

    String_Builder& string_builder_appendf(String_Builder& sb, const char* fmt, auto...args)
    {
        // In some cases, there is only a simple string in fmt, and no args.
        // We get a warning from sprintf called inside string_printf
        if constexpr (sizeof...(args) == 0)
        {
            return string_builder_append(sb, fmt);
        }
        else
        {
            const String formatted_str = string_tprintf(fmt, args...);
            return string_builder_append(sb, formatted_str);
        }
    }

    inline void string_builder_release(String_Builder& sb)
    {
        // sb.buffer is temporary
        sb.buffer.data = nullptr;
        sb.buffer.size = 0;
    }

    inline void string_builder_reset(String_Builder& sb)
    {
        array_resize(sb.buffer, 0);
    }

    inline String_Builder& string_builder_append(String_Builder& sb, const String& str)
    {
        // str could be destroyed before user call build_string
        String str_copy = string_tcopy(str);

        array_append(sb.buffer, str_copy );
        return sb;
    }

    inline String_Builder& string_builder_append(String_Builder& sb, const Array<String>& arr)
    {
        for(auto& each : arr)
        {
            string_builder_append(sb, each );
        }
        return sb;
    }

    inline String string_builder_build_tstring(String_Builder& sb, String separator)
    {
        push_allocator(&temp_allocator);
        String result = string_builder_build_string(sb, separator);
        pop_allocator();
        return result;
    }

    inline String string_builder_build_string(String_Builder& sb, String separator)
    {
        // compute the size of the output string
        u32_t size = 0;

        for( u32_t i = 0; i < sb.buffer.size; i++)
        {
            size += sb.buffer[i].size;
        }

        if( sb.buffer.size )
        {
            size += separator.size * (sb.buffer.size-1); // 1 separator after each, except last
        }

        if( size == 0)
        {
            return "";
        }

        size += 1; // +1 null-terminated

        // Initialize a string at the given length
        char* data = static_cast<char*>(allocator->proc_malloc(size)); 
        assert(data != nullptr);
        
        // Copy elem0 + separator + elem1 + ... + elemN-1
        char* cursor = data;
        for(u32_t i = 0; i < sb.buffer.size; ++i)
        {
            if ( separator.size != 0 && i != 0 )
            {
                std::memcpy(cursor, separator.data, separator.size);
                cursor += separator.size;
            }
            std::memcpy(cursor,  sb.buffer[i].data, sb.buffer[i].size);
            cursor += sb.buffer[i].size;
        }

        data[size-1] = '\0';

        string_builder_reset(sb);

        String result(data, size-1, String_Flags_IS_NULL_TERMINATED);

        return result;
    }
}