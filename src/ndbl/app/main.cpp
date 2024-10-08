#include "ndbl/gui/Nodable.h"
#include "tools/core/memory/memory.h"

using namespace tools;
using namespace ndbl;

int main(int argc, char *argv[])
{
    try_TOOLS_MAIN
    {
        Nodable app;
        app.init();
        while ( !app.should_stop() )
        {
            app.update();
            app.draw();
        }
        app.shutdown();
    }
    catch_TOOLS_MAIN
    return 0;
}
