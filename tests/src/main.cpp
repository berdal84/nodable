#include "gtest/gtest.h"
#include "Log.h"

using namespace Nodable::core;
using Verbosity = Log::Verbosity;

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
//    Log::SetVerbosityLevel("Parser", Verbosity::Verbose);
//    Log::SetVerbosityLevel("GraphNode", Verbosity::Verbose);
    return RUN_ALL_TESTS();
}