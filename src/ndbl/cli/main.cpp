#include "CLI.h"
#include "tools/core/memory/MemoryManager.h"

using tools::log;
using namespace ndbl;

int main(int argc, char *argv[])
{
    try_TOOLS_MAIN
    {
        tools::log::set_verbosity(log::Verbosity_Warning );

        ndbl::CLI cli;
        cli.init();
        while ( !cli.should_stop() )
        {
            cli.update();
        }
        cli.shutdown();
    }
    catch_TOOLS_MAIN
}
