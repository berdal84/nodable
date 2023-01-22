#include <nodable/app/App.h>
#include <nodable/app/AppView.h>

using Verbosity = fw::Log::Verbosity;

int main(int argc, char *argv[])
{
    // by default log is on Verbosity::Message ...

//    nodable::Log::SetVerbosityLevel("R", Verbosity::Verbose);
//    nodable::Log::SetVerbosityLevel("File", Verbosity::Message);
//    nodable::Log::SetVerbosityLevel("GraphTraversal", Verbosity::Message);
//    nodable::Log::SetVerbosityLevel("Parser", Verbosity::Message);
//    nodable::Log::SetVerbosityLevel("GraphNode", Verbosity::Message);
//    nodable::Log::SetVerbosityLevel("GraphNodeView", Verbosity::Message);
//    nodable::Log::SetVerbosityLevel("VM", Verbosity::Message);

    // by default log is on Verbosity::Message ...

    fw::type_register::log_statistics();

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
