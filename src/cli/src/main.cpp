#include <nodable/cli/CLI.h>
#include <nodable/core/Log.h>

using Verbosity = ndbl::Log::Verbosity;

int main(int argc, char *argv[])
{
    // by default log is on Verbosity::Message ...

//    ndbl::Log::SetVerbosityLevel("R", Verbosity::Verbose);
//    ndbl::Log::SetVerbosityLevel("File", Verbosity::Message);
//    ndbl::Log::SetVerbosityLevel("GraphTraversal", Verbosity::Message);
//    ndbl::Log::SetVerbosityLevel("Parser", Verbosity::Message);
//    ndbl::Log::SetVerbosityLevel("GraphNode", Verbosity::Message);
//    ndbl::Log::SetVerbosityLevel("GraphNodeView", Verbosity::Message);
//    ndbl::Log::SetVerbosityLevel("VM", Verbosity::Message);

    // by default log is on Verbosity::Message ...

    ndbl::CLI cli;

    while (!cli.should_stop())
    {
        cli.update();
    }

    LOG_FLUSH()
    return 0;
}
