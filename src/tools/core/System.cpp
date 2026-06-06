#include "System.h"
#include <cstdlib>    // for ::system
#include <thread>     // for std::thread
#include "Log.h"

using namespace tools;

#ifdef NDBL_DESKTOP
int tools::system_run_command(const char* command)
{
    int exit_code = ::system(command);
    if ( exit_code != 0 )
    {
        TOOLS_LOG(tools::Verbosity_Error, "tools::system", "Command failed: %s", command);
    }
    return exit_code;
};

void tools::system_open_url_async(std::string url)
{
    std::string command = "x-www-browser " + url; // TODO: does not work on all distros
    std::thread thread( system_run_command, command.c_str() );
    thread.detach();
}

void tools::system_clear_console() /* cf: https://stackoverflow.com/questions/6486289/how-can-i-clear-console */
{
    if( std::system("clear") )
    {
        TOOLS_LOG(tools::Verbosity_Error, "System", "Unable to reset console");
    }
}

#elif __EMSCRIPTEN__
#include <emscripten.h>

EM_JS(void, call_clear_console, (), {
  alert('call_clear_console not implemented yet');
  throw 'all done';
});

EM_JS(void, call_open_url, (), {
  alert('call_open_url not implemented yet');
  throw 'all done';
});

void system_open_url_async(std::string url)
{
    call_open_url();
}

void system_clear_console() /* cf: https://stackoverflow.com/questions/6486289/how-can-i-clear-console */
{
    call_clear_console();
}

#endif
