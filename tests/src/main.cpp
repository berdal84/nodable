#include <gtest/gtest.h>
#include <nodable/core/Log.h>
#include <nodable/core/reflection/reflection>

using namespace Nodable;
using Verbosity = Log::Verbosity;

int main(int argc, char **argv) {

    ::testing::InitGoogleTest(&argc, argv);

    Log::SetVerbosityLevel("Reflect"  , Verbosity::Message);
    Log::SetVerbosityLevel("Language" , Verbosity::Message);
    Log::SetVerbosityLevel("GraphNode", Verbosity::Message);
    Log::SetVerbosityLevel("Parser"   , Verbosity::Message);
    Log::SetVerbosityLevel("VM"       , Verbosity::Message);
    Log::SetVerbosityLevel("VM::CPU"  , Verbosity::Message);

    typeregister::log_statistics();

    return RUN_ALL_TESTS();
}