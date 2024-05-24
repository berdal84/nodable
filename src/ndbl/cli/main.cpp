#include "CLI.h"

using tools::log;
using namespace ndbl;

int main(int argc, char *argv[])
{
    // 1) configure tools related stuff
    //---------------------------------

    // Adjust lo levels
    tools::log::set_verbosity(log::Verbosity_Warning );

    // 2) Instantiate and run the app
    //-------------------------------
    ndbl::CLI cli;
    cli.init();
    while ( !cli.should_stop() )
    {
        cli.update();
    }
    cli.shutdown();
}
