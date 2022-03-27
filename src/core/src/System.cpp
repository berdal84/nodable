#include <nodable/core/System.h>

#include <whereami/src/whereami.h> // to locate executable directory
#include <cstdlib>                 // abort()
#include <thread>                  // std::thread

#include <nodable/core/Log.h>

using namespace Nodable;

void System::open_url_async(std::string _URL)
{
    auto open_url = [](std::string _URL)
    {
#ifdef WIN32
        std::string command("start");
#elif __APPLE__
        std::string command("open");
#elif __linux__
        std::string command("x-www-browser");
#else
#       error "Unknown operating system."
#endif

        std::string op = command + " " + _URL;
        auto result = system(op.c_str());

        if (result != 0)
        {
            LOG_ERROR( "System", "Unable to open %s. Because the command %s is not available on your system.",
                       _URL.c_str(), command.c_str())
        }

        return result;
    };

    std::thread(open_url, _URL).detach();
}

std::string System::get_executable_directory()
{
    std::string result;

    // set asset absolute path
    char* path = nullptr;
    int length, dirname_length;
    length = wai_getExecutablePath(nullptr, 0, &dirname_length);
    if (length > 0)
    {
        path = new char[length + 1];

        if ( wai_getExecutablePath(path, length, &dirname_length) )
        {
            path[length] = '\0';
            LOG_MESSAGE("System", "executable path: %s\n", path);
            path[dirname_length] = '\0';
            LOG_MESSAGE("System", "  dirname: %s\n", path);
            LOG_MESSAGE("System", "  basename: %s\n", path + dirname_length + 1);
            result.append(path);
        }
        else
        {
            LOG_ERROR("System", "Unable to get executable path\n");
        }
        delete path;
    }
    else
    {
        LOG_WARNING("System", "Unable to get executable directory!\n");
    }
    return result;
}
