#include <nodable/app/App.h>
#include <nodable/app/AppView.h>

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

    ndbl::type_register::log_statistics();

    ndbl::App app;

    if( app.init() )
    {
        while (!app.should_stop())
        {
            app.update();
            app.draw();
        }
    }
    app.shutdown();
    LOG_FLUSH()
    return 0;
}
