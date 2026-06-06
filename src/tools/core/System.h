#pragma once
#include <string>

namespace tools
{
    extern void system_open_url_async(std::string /* url */); // Browse a given URL asynchronously
    extern void system_clear_console();
    extern int  system_run_command(const char* command);
}
