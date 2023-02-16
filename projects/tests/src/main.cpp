#include <gtest/gtest.h>
#include "fw/core/log.h"
#include <fw/core/reflection/reflection>

using namespace fw;

int main(int argc, char **argv) {

    ::testing::InitGoogleTest(&argc, argv);

    log::set_verbosity("Reflect"  , log::Verbosity_Message);
    log::set_verbosity("Language" , log::Verbosity_Message);
    log::set_verbosity("NodableLanguage" , log::Verbosity_Verbose);
    log::set_verbosity("GraphNode", log::Verbosity_Message);
    log::set_verbosity("Parser"   , log::Verbosity_Message);
    log::set_verbosity("VM"       , log::Verbosity_Message);
    log::set_verbosity("VM::CPU"  , log::Verbosity_Message);

    type_register::log_statistics();

    return RUN_ALL_TESTS();
}