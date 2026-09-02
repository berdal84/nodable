#include "Format.h"
#include "bdc/Allocators.hpp"
#include "bdc/String.hpp"
#include "bdc/String_Builder.hpp"
#include <chrono>
#include <ctime>


namespace tools
{
    using namespace bdc;

    String Format::number(double d)
    {
        String result = string_printf(temp_allocator(), "%#llx", d);
        limit_trailing_zeros(result, 1);
        return result;
    }

    String Format::hexadecimal(u64_t _addr)
    {
        return string_printf(temp_allocator(), "%#llx", _addr);
    }

    String Format::address(const void* _addr)
    {
        return string_printf(temp_allocator(), "%p", _addr);
    }

    void Format::limit_trailing_zeros(String& str, int _trailing_max)
    {
        // limit to _trailing_max zeros
        size_t first_zero_to_remove = string_rfind(str, '.') + 1 + _trailing_max;
        
        string_lsplit(str, first_zero_to_remove);

        // erase eventual dot
        if ( _trailing_max == 0 && str[str.size-1] == '.')
        {
            str.size -= 1;
        }

        if( str[str.size-1] == '.')
        {
            str[str.size] = '\0';
            str.size += 1;
        }

    }

    String Format::time_point_to_string(const std::chrono::system_clock::time_point &time_point)
    {
        std::time_t time = std::chrono::system_clock::to_time_t(time_point);
        std::tm* tm_info = std::localtime(&time);

        String result{};
        result.data  = memory_malloc_array<char>( 32, temp_allocator() );
        result.size  = std::strftime(result.data, 31, "%Y-%m-%d %H:%M:%S", tm_info);
        result.flags = String_Flags_IS_NULL_TERMINATED;

        return result; 
    }
        
    String tools::Format::title(const String& title, u32_t width) // Format a title for console output (ex: ------<=[ My Title ]=>--------)
    {
        /*
        * Takes _title and do:
        * ------------<=[ _title ]=>------------
        */

        String pre       = "-=[ ";
        String post      = " ]=-";
        String padding   = "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=";
        padding.size = (width - title.size - pre.size - post.size) / 2;

        String_Builder sb{};
        string_builder_init(sb);
        string_builder_appendf(sb, "%s%s%s%s%s", padding.c_str(), pre.c_str(), title.c_str(), post.c_str(), padding.c_str() );
        
        String result = string_builder_build_string(sb);
        return result;
    }
} // namespace bdc