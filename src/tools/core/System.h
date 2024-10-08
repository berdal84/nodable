#pragma once
#include <string>

namespace tools
{
    namespace System // multi-platform static functions
    {
        extern void  open_url_async(std::string /* url */); // Browse a given URL asynchronously
        extern void  clear_console();
    };
}
