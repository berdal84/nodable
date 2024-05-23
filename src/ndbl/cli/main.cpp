#include "CLI.h"
#include "ndbl/core/core.h"

using tools::log;

int main(int argc, char *argv[])
{
    ndbl::core_init();
    log::set_verbosity(log::Verbosity_Warning );
    {
        ndbl::CLI cli;

        while ( !cli.should_stop() )
        {
            cli.update();
        }
    }
    ndbl::core_shutdown();
    LOG_FLUSH()
    return 0;
}
