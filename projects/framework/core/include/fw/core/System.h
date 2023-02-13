#pragma once
#include <string>

namespace fw
{
    namespace System                                    // Set of cross-platform functions library to deal with system
    {
        extern void        open_url_async(std::string); // Browse a given URL asynchronously
        extern std::string get_executable_directory();  // Get the path to the executable directory

        namespace console                               //  Set of cross-platform functions library to deal with the console
        {
            extern void clear();                        // Clear the console
        }
    };
}
