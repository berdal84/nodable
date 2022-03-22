
#include <string>

#include <nodable/app/App.h>
#include <nodable/app/AppView.h>

using Verbosity = Nodable::Log::Verbosity;

int main(int argc, char *argv[])
{
    Nodable::Log::SetVerbosityLevel("R", Verbosity::Verbose);
    Nodable::Log::SetVerbosityLevel("File", Verbosity::Message);
    Nodable::Log::SetVerbosityLevel("GraphTraversal", Verbosity::Message);
    Nodable::Log::SetVerbosityLevel("Parser", Verbosity::Message);
    Nodable::Log::SetVerbosityLevel("GraphNode", Verbosity::Message);
    Nodable::Log::SetVerbosityLevel("GraphNodeView", Verbosity::Message);
    Nodable::Log::SetVerbosityLevel("VM", Verbosity::Message);

    Nodable::App app;

    if( app.init() )
    {
        app.open_file(app.get_absolute_asset_path("txt/startup.txt") );

        while (!app.should_stop())
        {
            try
            {
                app.update();
            }
            catch (std::exception &err)
            {
                LOG_ERROR("main", "Unable to update application, reason: %s\n", err.what())
            }

            try
            {
                app.draw();
            }
            catch (std::exception &err)
            {
                LOG_ERROR("main", "Unable to draw application view, reason: %s\n", err.what())
            }

        }
    }
    app.shutdown();
    LOG_FLUSH()
    return 0;
}
