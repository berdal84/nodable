#include <ndbl/cli/CLI.h>
#include "fw/core/log.h"

using Verbosity = fw::log::Verbosity;

int main(int argc, char *argv[])
{
    // by default log is on Verbosity::Message ...

//    fw::Log::SetVerbosityLevel("R", Verbosity::Verbose);
//    fw::Log::SetVerbosityLevel("File", Verbosity::Message);
//    fw::Log::SetVerbosityLevel("GraphTraversal", Verbosity::Message);
//    fw::Log::SetVerbosityLevel("Parser", Verbosity::Message);
//    fw::Log::SetVerbosityLevel("GraphNode", Verbosity::Message);
//    fw::Log::SetVerbosityLevel("GraphNodeView", Verbosity::Message);
//    fw::Log::SetVerbosityLevel("VM", Verbosity::Message);

    // by default log is on Verbosity::Message ...

    ndbl::CLI cli;

    while (!cli.should_stop())
    {
        cli.update();
    }

    LOG_FLUSH()
    return 0;
}
