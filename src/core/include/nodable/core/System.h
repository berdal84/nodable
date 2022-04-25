#pragma once
#include <string>

namespace ndbl
{
    /**
     * Static library cross platform to deal with system
     */
    namespace System
    {
        extern void        open_url_async(std::string /* url */);
        extern std::string get_executable_directory();

        namespace console
        {
            extern void clear();
        }

#if (WIN32)
        static constexpr char  k_end_of_line = '\n';
#elif(__APPLE__ || __linux__)
        static constexpr char  k_end_of_line = '\n'; // <--- for now we use the same convention.
#endif

    };
}
