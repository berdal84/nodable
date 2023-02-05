#include <gtest/gtest.h>
#include "fw/core/Log.h"
#include <fw/core/reflection/reflection>

using namespace fw;

int main(int argc, char **argv) {

    ::testing::InitGoogleTest(&argc, argv);

    Log::set_verbosity("Reflect"  , Log::Verbosity_Message);
    Log::set_verbosity("Language" , Log::Verbosity_Message);
    Log::set_verbosity("NodableLanguage" , Log::Verbosity_Verbose);
    Log::set_verbosity("GraphNode", Log::Verbosity_Message);
    Log::set_verbosity("Parser"   , Log::Verbosity_Message);
    Log::set_verbosity("VM"       , Log::Verbosity_Message);
    Log::set_verbosity("VM::CPU"  , Log::Verbosity_Message);

    type_register::log_statistics();

    return RUN_ALL_TESTS();
}