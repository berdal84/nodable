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
    };
}
