#include "ndbl/gui/index.h"

using namespace tools;
using namespace ndbl;

int main(int argc, char *argv[])
{
    ndbl::init_reflection_with_gui();

    TOOLS_try
    {
        Nodable app;
        app.init();
        app.run();
        app.shutdown();
    }
    TOOLS_catch
    return 0;
}
