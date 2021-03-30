#include "gtest/gtest.h"
#include "Core/Log.h"

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    Nodable::Log::SetVerbosityLevel("Parser", Nodable::Log::Verbosity::Verbose);
    Nodable::Log::SetVerbosityLevel("GraphNode", Nodable::Log::Verbosity::Verbose);
    return RUN_ALL_TESTS();
}