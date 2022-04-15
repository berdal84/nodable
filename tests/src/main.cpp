#include <gtest/gtest.h>
#include <nodable/core/Log.h>
#include <nodable/core/reflection/reflection>

using namespace Nodable;
using Verbosity = Log::Verbosity;

int main(int argc, char **argv) {

    ::testing::InitGoogleTest(&argc, argv);

    Log::set_verbosity("Reflect", Verbosity::Message);
    Log::set_verbosity("Language", Verbosity::Message);
    Log::set_verbosity("GraphNode", Verbosity::Message);
    Log::set_verbosity("Parser", Verbosity::Message);
    Log::set_verbosity("VM", Verbosity::Message);
    Log::set_verbosity("VM::CPU", Verbosity::Message);

    typeregister::log_statistics();

    return RUN_ALL_TESTS();
}