#include "ndbl/gui/App.h"
#include "ndbl/gui/AppView.h"

int main(int argc, char *argv[])
{
    fw::type_register::log_statistics();

    ndbl::App app;

    if (app.init())
    {
        while (!app.should_stop())
        {
            app.update();
            app.draw();
        }
        app.shutdown();
        LOG_FLUSH()
        return 0;
    }
    else
    {
        LOG_FLUSH()
        return 1;
    }
}
