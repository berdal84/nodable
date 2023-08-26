#include "nodable/gui/Nodable.h"

int main(int argc, char *argv[])
{
#ifdef NDBL_DEBUG
    fw::log::set_verbosity(fw::log::Verbosity_Warning);
    fw::log::set_verbosity("Pool", fw::log::Verbosity_Verbose);
#endif
    ndbl::Nodable app;            // Instantiate app
    return app.main(argc, argv);  // Run main loop
}
