#include <gtest/gtest.h>
#include <nodable/Log.h>
#include <nodable/R.h>

using namespace Nodable;
using Verbosity = Log::Verbosity;

int main(int argc, char **argv) {

    ::testing::InitGoogleTest(&argc, argv);

    Log::SetVerbosityLevel("Reflect", Verbosity::Warning);
    Log::SetVerbosityLevel("Language", Verbosity::Warning);
    Log::SetVerbosityLevel("GraphNode", Verbosity::Warning);
    Log::SetVerbosityLevel("Parser", Verbosity::Warning);
    Log::SetVerbosityLevel("VM", Verbosity::Verbose);

    R::Initialize();

    return RUN_ALL_TESTS();
}