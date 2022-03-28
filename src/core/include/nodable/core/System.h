#pragma once
#include <string>

namespace Nodable
{
    /**
     * Static library cross platform to deal with system
     */
    class System
    {
    public:
        static void        open_url_async(std::string /* url */);
        static std::string get_executable_directory();

#if (WIN32)
        static constexpr char  k_end_of_line = '\n';
#elif(__APPLE__ || __linux__)
        static constexpr char  k_end_of_line = '\n'; // <--- for now we use the same convention.
#endif

    };
}
