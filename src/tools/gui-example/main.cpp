#include "AppExample.h"
#include "tools/core/memory/MemoryManager.h"

using namespace tools;

int main(int argc, char *argv[])
{
#ifdef TOOLS_DEBUG
    init_memory_manager();
#endif
    // Instantiate the application using the predefined configuration
    AppExample app;
    app.init();

    while( !app.should_stop )
    {
        app.update();
        app.draw();
    }
    app.shutdown();
#ifdef TOOLS_DEBUG
    shutdown_memory_manager();
#endif
}
