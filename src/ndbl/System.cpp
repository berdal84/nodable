#include "System.h"

#include <cstdlib>    // for ::system
#include <thread>     // for std::thread

#include "bdc/Allocators.hpp"
#include "bdc/String.hpp"
#include "Log.h"

#ifdef NDBL_DESKTOP

namespace ndbl
{
int system_run_command(const char* command)
{
    int exit_code = ::system( command );
    if ( exit_code != 0 )
    {
        NDBL_LOG(Verbosity_Error, "system", "Command failed: %s", command );
    }
    return exit_code;
};

void system_open_url_async(const char* url)
{
    bdc::String command = bdc::string_tprintf( "x-www-browser %s", url);
    std::thread thread( system_run_command, command.data );
    thread.detach();
}

void system_clear_console() /* cf: https://stackoverflow.com/questions/6486289/how-can-i-clear-console */
{
    if( std::system("clear") )
    {
        NDBL_LOG(Verbosity_Error, "System", "Unable to reset console");
    }
}

} // namespace ndbl

#elif __EMSCRIPTEN__
#include <emscripten.h>

namespace ndbl
{

EM_JS(void, call_clear_console, (), {
  alert('call_clear_console not implemented yet');
  throw 'all done';
});

EM_JS(void, call_open_url, (), {
  alert('call_open_url not implemented yet');
  throw 'all done';
});

void system_open_url_async(const bdc::String& url)
{
    call_open_url();
}

void system_clear_console() /* cf: https://stackoverflow.com/questions/6486289/how-can-i-clear-console */
{
    call_clear_console();
}

} // namespace ndbl

#endif
