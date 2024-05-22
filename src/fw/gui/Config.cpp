#include "Config.h"

fw::Config& fw::g_conf()
{
    static fw::Config conf;
    return conf;
};