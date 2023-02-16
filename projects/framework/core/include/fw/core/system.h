#pragma once
#include <string>

namespace fw
{
    class system // multi-platform static functions
    {
    public:
        static void        open_url_async(std::string /* url */); // Browse a given URL asynchronously
        static std::string get_executable_directory();            // Get the executable directory absolute path

        class console
        {
        public: static void clear();
        };
    };
}
