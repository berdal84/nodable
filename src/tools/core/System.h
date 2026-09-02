#pragma once

// forward declarations
namespace bdc
{
    struct String;
}

namespace tools
{
    void system_open_url_async(const bdc::String& /* url */); // Browse a given URL asynchronously
    void system_clear_console();
    int  system_run_command(const bdc::String& /* command */);
}
