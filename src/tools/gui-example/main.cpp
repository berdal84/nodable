#include "AppExample.h"
#include "tools/core/memory/MemoryManager.h"

using namespace tools;

int main(int argc, char *argv[])
{
    TOOLS_DEBUG_TRY
    {
        // Instantiate the application using the predefined configuration
        AppExample app;
        app.init();

        while( !app.should_stop )
        {
            app.update();
            app.draw();
        }
        app.shutdown();
    }
    TOOLS_DEBUG_CATCH
}
