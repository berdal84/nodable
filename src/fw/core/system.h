#pragma once
#include <string>
#include <filesystem>

namespace fw
{
    namespace system // multi-platform static functions
    {
        extern void                  open_url_async(std::string /* url */); // Browse a given URL asynchronously
        extern std::filesystem::path get_executable_directory();            // Get the executable directory absolute path
        extern void clear_console();
    };
}
