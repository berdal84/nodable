#include <ndbl/gui/Nodable.h>

int main(int argc, char *argv[])
{
    fw::type_register::log_statistics(); // log statistics relative to the reflection system

    ndbl::Nodable app; // Instantiate app
    return app.run();  // Run main loop
}
