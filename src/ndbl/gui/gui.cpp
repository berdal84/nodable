#include "gui.h"
#include "tools/gui/gui.h"

ndbl::Config* ndbl::g_conf{nullptr};

void ndbl::gui_init()
{
    LOG_MESSAGE("ndbl", "gui_init() ..\n");
    EXPECT(ndbl::g_conf == nullptr, "ndbl::g_cong is already initialized")
    tools::gui_init();
    ndbl::g_conf = new ndbl::Config();
    LOG_MESSAGE("ndbl", "gui_init() OK\n");
};

void ndbl::gui_shutdown()
{
    LOG_MESSAGE("ndbl", "gui_shutdown() ..\n");
    EXPECT(ndbl::g_conf != nullptr, "No g_conf is initialized, did you call tools::shutdown() twice?\n")
    tools::gui_shutdown();
    delete ndbl::g_conf;
    ndbl::g_conf = nullptr;
    LOG_MESSAGE("ndbl", "gui_shutdown() OK\n");
};