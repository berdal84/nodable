#include "gui.h"

fw::Config* fw::g_conf{nullptr};

void fw::init()
{
    FW_EXPECT(fw::g_conf == nullptr, "fw::g_conf is already initialized\n")
    fw::g_conf = new Config();

    Pool::init();
};

void fw::shutdown()
{
    FW_EXPECT(fw::g_conf != nullptr, "fw::g_conf was not initialized, did you call fw::shutdown() twice?\n")
    delete fw::g_conf;
    fw::g_conf = nullptr;

    Pool::shutdown();
};