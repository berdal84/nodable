#include "ndbl/gui/Config.h"
#include "ndbl/gui/Nodable.h"

using namespace tools;
using namespace ndbl;

int main(int argc, char *argv[])
{
#ifdef NDBL_DEBUG
    log::set_verbosity(log::Verbosity_Warning);
    log::set_verbosity("Pool", log::Verbosity_Verbose);
#endif
    Nodable app;
    app.init();
    while ( !app.should_stop )
    {
        app.update();
        app.draw();
    }
    app.shutdown();
}
