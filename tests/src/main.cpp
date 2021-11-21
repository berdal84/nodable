#include <gtest/gtest.h>
#include <nodable/Log.h>

using namespace Nodable;
using Verbosity = Log::Verbosity;

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    Log::SetVerbosityLevel("Reflect", Verbosity::Verbose);
//    Log::SetVerbosityLevel("Parser", Verbosity::Verbose);
    Log::SetVerbosityLevel("GraphNode", Verbosity::Verbose);
    return RUN_ALL_TESTS();
}