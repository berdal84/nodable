#include "gtest/gtest.h"
#include "../../src/core/include/nodable/Log.h"

using namespace Nodable;
using Verbosity = Log::Verbosity;

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
//    Log::SetVerbosityLevel("Parser", Verbosity::Verbose);
//    Log::SetVerbosityLevel("GraphNode", Verbosity::Verbose);
    return RUN_ALL_TESTS();
}