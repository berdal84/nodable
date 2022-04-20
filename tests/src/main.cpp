#include <gtest/gtest.h>
#include <nodable/core/Log.h>
#include <nodable/core/reflection/type_register.h>

using namespace Nodable;

int main(int argc, char **argv) {

    ::testing::InitGoogleTest(&argc, argv);

    Log::set_verbosity("Reflect"  , Log::Verbosity_Message);
    Log::set_verbosity("Language" , Log::Verbosity_Message);
    Log::set_verbosity("GraphNode", Log::Verbosity_Message);
    Log::set_verbosity("Parser"   , Log::Verbosity_Message);
    Log::set_verbosity("VM"       , Log::Verbosity_Message);
    Log::set_verbosity("VM::CPU"  , Log::Verbosity_Message);

    type_register::log_statistics();

    return RUN_ALL_TESTS();
}