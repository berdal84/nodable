#include <nodable/cli/CLI.h>
#include <nodable/core/Log.h>

using Verbosity = Nodable::Log::Verbosity;

int main(int argc, char *argv[])
{
    // by default log is on Verbosity::Message ...

//    Nodable::Log::SetVerbosityLevel("R", Verbosity::Verbose);
//    Nodable::Log::SetVerbosityLevel("File", Verbosity::Message);
//    Nodable::Log::SetVerbosityLevel("GraphTraversal", Verbosity::Message);
//    Nodable::Log::SetVerbosityLevel("Parser", Verbosity::Message);
//    Nodable::Log::SetVerbosityLevel("GraphNode", Verbosity::Message);
//    Nodable::Log::SetVerbosityLevel("GraphNodeView", Verbosity::Message);
//    Nodable::Log::SetVerbosityLevel("VM", Verbosity::Message);

    // by default log is on Verbosity::Message ...

    Nodable::CLI cli;

    while (!cli.should_stop())
    {
        cli.update();
    }

    LOG_FLUSH()
    return 0;
}
