#include "ndbl/gui/Nodable.h"
#include "tools/core/memory/MemoryManager.h"

using namespace tools;
using namespace ndbl;

int main(int argc, char *argv[])
{
    TOOLS_DEBUG_TRY
    {
        Nodable app;
        app.init();
        while ( !app.should_stop )
        {
            app.update();
            app.draw();
        }
        app.shutdown();
    }
    TOOLS_DEBUG_CATCH
}
