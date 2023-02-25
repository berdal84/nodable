#pragma once
#include <string>
#include "ghc/filesystem.hpp"

namespace fw
{
    class system // multi-platform static functions
    {
    public:
        static void                  open_url_async(std::string /* url */); // Browse a given URL asynchronously
        static ghc::filesystem::path get_executable_directory();            // Get the executable directory absolute path

        class console
        {
        public: static void clear();
        };
    };
}
