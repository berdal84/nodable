#pragma once
#include "String.hpp"
#include "Allocators.hpp"
#include "Types.hpp"

namespace bdc
{
    struct String_Builder
    {
        Resizable_Array<String> buffer;
        Allocator*              allocator; // will be used for data.allocator and any append / appendf
    };
    static_assert( std::is_trivially_constructible_v<String_Builder> );

    void            string_builder_init(String_Builder&);
    void            string_builder_release(String_Builder&);
    String_Builder& string_builder_append(String_Builder&, const String& str);
    String_Builder& string_builder_append(String_Builder&, const Resizable_Array<String>& arr);
    String_Builder& string_builder_appendf(String_Builder& sb, const char* fmt, auto...args);
    String          string_builder_build_string(String_Builder&, Allocator* allocator = temp_allocator() );             
    String          string_builder_build_string(String_Builder&, String separator, Allocator* allocator = temp_allocator() );

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
            const String result = string_printf(sb.allocator, fmt, args...);
            return string_builder_append(sb, result);
        }
    }
}