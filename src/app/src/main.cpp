
#include <string>

#include <nodable/App.h>
#include <nodable/AppView.h>
#include <nodable/BuildInfo.h>

using namespace Nodable;

int main(int argc, char *argv[])
{
    Log::SetVerbosityLevel("R", Log::Verbosity::Verbose);
    Log::SetVerbosityLevel("File", Log::Verbosity::Message);
    Log::SetVerbosityLevel("GraphTraversal", Log::Verbosity::Message);
    Log::SetVerbosityLevel("Parser", Log::Verbosity::Message);
    Log::SetVerbosityLevel("GraphNode", Log::Verbosity::Message);
    Log::SetVerbosityLevel("GraphNodeView", Log::Verbosity::Message);
    Log::SetVerbosityLevel("VM", Log::Verbosity::Message);

    App app(BuildInfo::version_extended.c_str());
    app.init();
    auto startupFilePath = app.get_asset_path("txt/startup.txt");
    app.open_file(startupFilePath); // Init and open a startup file

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


        if (AppView *view = app.get_view())
        {
            try
            {
                view->draw();
            }catch (std::exception &err)
            {
                LOG_ERROR("main", "Unable to draw application view, reason: %s\n", err.what())
            }
        }
    }

    app.shutdown();
    LOG_FLUSH()
    return 0;
}
