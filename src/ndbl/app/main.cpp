#include "ndbl/gui/Nodable.h"
#include "tools/core/memory/MemoryManager.h"

using namespace tools;
using namespace ndbl;

int main(int argc, char *argv[])
{
#ifdef TOOLS_DEBUG
    init_memory_manager();
#endif
    Nodable app;
    app.init();
    while ( !app.should_stop )
    {
        app.update();
        app.draw();
    }
    app.shutdown();
#ifdef TOOLS_DEBUG
    shutdown_memory_manager();
#endif
}
