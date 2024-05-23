#include "AppExample.h"

using namespace tools;

int main(int argc, char *argv[])
{
    // Instantiate the application using the predefined configuration
    AppExample app;
    app.init();

    while( !app.should_stop )
    {
        app.update();
        app.draw();
    }
    app.shutdown();
}
