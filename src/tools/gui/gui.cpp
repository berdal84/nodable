#include "gui.h"
#include "tools/core/core.h"

tools::Config* tools::g_conf{nullptr};

void tools::gui_init()
{
    LOG_MESSAGE("tools", "gui_init() ...\n");
    EXPECT( g_conf == nullptr, "tools::g_conf is already initialized\n")
    g_conf = new Config();

    Pool::init();
    LOG_MESSAGE("tools", "gui_init() OK\n");
};

void tools::gui_shutdown()
{
    LOG_MESSAGE("tools", "gui_shutdown() ..\n");
    EXPECT( g_conf != nullptr, "tools::g_conf was not initialized, did you call tools::shutdown() twice?\n")
    delete g_conf;
    g_conf = nullptr;

    tools::core_shutdown();
    LOG_MESSAGE("tools", "gui_shutdown() OK\n");
};