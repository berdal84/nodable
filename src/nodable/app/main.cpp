#include "nodable/gui/Nodable.h"

using namespace fw;
using namespace ndbl;

int main(int argc, char *argv[])
{
#ifdef NDBL_DEBUG
    log::set_verbosity(log::Verbosity_Warning);
    log::set_verbosity("Pool", log::Verbosity_Verbose);
#endif
    Nodable app;                   // Instantiate app
    return app.main(argc, argv);  // Run main loop
}
