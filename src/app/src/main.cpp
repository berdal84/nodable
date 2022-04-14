#include <nodable/app/App.h>
#include <nodable/app/AppView.h>

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

    Nodable::typeregister::log_statistics();

    Nodable::App app;

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
