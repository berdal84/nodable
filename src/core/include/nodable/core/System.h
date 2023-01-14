#pragma once
#include <string>

namespace ndbl
{
    /**
     * @namespace Set of cross platform functions library to deal with system
     */
    namespace System
    {
        /** Browse a given URL asynchronously */
        extern void        open_url_async(std::string /* url */);
        /** Get the path to the executable directory */
        extern std::string get_executable_directory();

        /**
         * @namespace Set of cross platform functions library to deal with the console
         */
        namespace console
        {
            /** Clear the console */
            extern void clear();
        }

#if (WIN32)
        static constexpr char  k_end_of_line = '\n';
#elif(__APPLE__ || __linux__)
        static constexpr char  k_end_of_line = '\n'; // <--- for now we use the same convention.
#endif

    };
}
