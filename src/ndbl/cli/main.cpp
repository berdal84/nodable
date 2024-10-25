#include "CLI.h"
#include "tools/core/TryCatch.h"

using tools::log;
using namespace ndbl;

int main(int argc, char *argv[])
{
    TOOLS_try
    {
        ndbl::CLI cli;
        cli.init();
        while ( !cli.should_stop() )
        {
            cli.update();
        }
        cli.shutdown();
    }
    TOOLS_catch
    return 0;
}
