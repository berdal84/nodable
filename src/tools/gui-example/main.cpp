#include "AppExample.h"
#include "tools/core/memory/MemoryManager.h"

using namespace tools;

int main(int argc, char *argv[])
{
    try_TOOLS_MAIN
    {
        // Instantiate the application using the predefined configuration
        AppExample app;
        app.init();

        while( !app.should_stop() )
        {
            app.update();
            app.draw();
        }
        app.shutdown();
    }
    catch_TOOLS_MAIN
}
