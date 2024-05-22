#include "gui.h"
#include "fw/gui/gui.h"

ndbl::Config* ndbl::g_conf{nullptr};

void ndbl::init()
{
    FW_EXPECT(ndbl::g_conf == nullptr, "ndbl::g_cong is already initialized")

    fw::init();
    ndbl::g_conf = new ndbl::Config();
};

void ndbl::shutdown()
{
    FW_EXPECT(ndbl::g_conf != nullptr, "No g_conf is initialized, did you call fw::shutdown() twice?\n")
    fw::shutdown();
    delete ndbl::g_conf;
    ndbl::g_conf = nullptr;
};