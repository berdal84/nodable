#include "System.h"
#include <cstdlib>    // for ::system
#include <thread>     // for std::thread
#include "Log.h"
#include "bdc/Allocators.hpp"
#include "bdc/String.hpp"

using namespace tools;

#ifdef NDBL_DESKTOP
int tools::system_run_command(const bdc::String& command)
{
    int exit_code = ::system(command.c_str() );
    if ( exit_code != 0 )
    {
        TOOLS_LOG(tools::Verbosity_Error, "tools::system", "Command failed: %s", command.c_str() );
    }
    return exit_code;
};

void tools::system_open_url_async(const bdc::String& url)
{
    bdc::String command = bdc::string_printf( bdc::temp_allocator(), "x-www-browser %s", url.c_str());
    std::thread thread( system_run_command, command );
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

void tools::system_open_url_async(const bdc::String url)
{
    call_open_url();
}

void tools::system_clear_console() /* cf: https://stackoverflow.com/questions/6486289/how-can-i-clear-console */
{
    call_clear_console();
}

#endif
