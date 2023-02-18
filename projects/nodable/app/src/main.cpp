#include "ndbl/gui/App.h"
#include "ndbl/gui/AppView.h"

int main(int argc, char *argv[])
{
    fw::type_register::log_statistics();

    ndbl::App app;
    return app.run();
}
