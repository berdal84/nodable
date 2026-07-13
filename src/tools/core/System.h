#pragma once

namespace tools
{
    void system_open_url_async(const char* /* url */); // Browse a given URL asynchronously
    void system_clear_console();
    int  system_run_command(const char* command);
}
