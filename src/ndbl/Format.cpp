#include "Format.h"

#include <chrono>
#include <ctime>

#include "bdc/Allocators.hpp"
#include "bdc/String.hpp"
#include "bdc/String_Builder.hpp"

namespace ndbl
{
    using namespace bdc;

    String Format::number(double d)
    {
        String result = string_tprintf("%#llx", d);
        limit_trailing_zeros(result, 1);
        return result;
    }

    String Format::hexadecimal(u64_t _addr)
    {
        return string_tprintf("%#llx", _addr);
    }

    String Format::address(const void* _addr)
    {
        return string_tprintf("%p", _addr);
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

    String Format::tprint_time_point(const std::chrono::system_clock::time_point &time_point)
    {
        std::time_t time = std::chrono::system_clock::to_time_t(time_point);
        std::tm* tm_info = std::localtime(&time);

        push_allocator(&temp_allocator);
        String result = new_string(32);
        pop_allocator();

        result.size = std::strftime(result.data, 31, "%Y-%m-%d %H:%M:%S", tm_info);

        return result; 
    }
        
    String Format::title(const char* title, u32_t width) // Format a title for console output (ex: ------<=[ My Title ]=>--------)
    {
        /*
        * Takes _title and do:
        * ------------<=[ _title ]=>------------
        */

        String pre       = string("-=[ ");
        String post      = string(" ]=-");
        String padding   = string("=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=");
        padding.size = (width - strlen(title) - pre.size - post.size) / 2;

        String_Builder sb{};
        string_builder_appendf(sb, "%s%s%s%s%s", c_str(padding), c_str(pre), title, c_str(post), c_str(padding) );
        
        String result = string_builder_build_tstring(sb);
        return result;
    }
} // namespace bdc