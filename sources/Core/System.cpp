#include "System.h"
#include "Log.h"
#include <thread>

void Nodable::System::OpenURL(std::string _URL)
{
    auto openURLambda = [](std::string _URL)
    {
#ifdef WIN32
        std::string command("start");
#else
        std::string command("x-www-browser");
#endif

        std::string op = command + " " + _URL;
        auto result = system(op.c_str());

        if (result != 0)
        {
            LOG_ERROR( Log::Verbosity::Normal, "Unable to open %s. Because the command %s is not available on your system.",
                       _URL.c_str(), command.c_str());
        }

        return result;
    };

    std::thread( openURLambda, _URL).detach();
}
