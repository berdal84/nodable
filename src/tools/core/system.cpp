#include "system.h"

#include <whereami.h> // to locate executable directory
#include <cstdlib>    // for ::system
#include <thread>     // for std::thread

#include "log.h"

using namespace tools;
using fs_path = std::filesystem::path;

void system::open_url_async(std::string _URL)
{
    auto open_url = [](std::string _URL)
    {
#ifdef WIN32
        std::string command("start");
#elif __APPLE__
        std::string command("open");
#elif __linux__
        std::string command("x-www-browser"); // TODO: does not work on all distros
#else
#       error "Unknown operating system."
#endif

        std::string op = command + " " + _URL;
        auto result = ::system(op.c_str());

        if (result != 0)
        {
            LOG_ERROR( "tools::system", "Unable to open %s. Because the command %s is not available on your system.",
                       _URL.c_str(), command.c_str())
        }

        return result;
    };

    std::thread(open_url, _URL).detach();
}

fs_path system::get_executable_directory()
{
    char* path = nullptr;
    int length, dirname_length;
    length = wai_getExecutablePath(nullptr, 0, &dirname_length);
    fs_path result;
    if (length > 0)
    {
        path = new char[length + 1];

        if ( wai_getExecutablePath(path, length, &dirname_length) )
        {
            path[length] = '\0';
            LOG_MESSAGE("tools::system", "executable path: %s\n", path);
            path[dirname_length] = '\0';
            LOG_MESSAGE("tools::system", "  dirname: %s\n", path);
            LOG_MESSAGE("tools::system", "  basename: %s\n", path + dirname_length + 1);
            result = path;
        }
        else
        {
            LOG_ERROR("tools::system", "Unable to get executable path\n");
        }
        delete[] path;
    }
    else
    {
        LOG_WARNING("tools::system", "Unable to get executable directory!\n");
    }
    return result;
}

void system::clear_console() /* cf: https://stackoverflow.com/questions/6486289/how-can-i-clear-console */
{
#if defined _WIN32
    const char* command = "cls";
#elif defined (__LINUX__) || defined(__gnu_linux__) || defined(__linux__) || defined (__APPLE__)
    const char* command = "clear";
#endif
    if(std::system(command)) LOG_ERROR("tools::system::console", "Unable to clear console");
}
