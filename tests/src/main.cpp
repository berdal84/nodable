#include <gtest/gtest.h>
#include <nodable/core/Log.h>
#include <nodable/core/reflection/R.h>

using namespace Nodable;
using Verbosity = Log::Verbosity;

int main(int argc, char **argv) {

    ::testing::InitGoogleTest(&argc, argv);

    Log::SetVerbosityLevel("Reflect", Verbosity::Warning);
    Log::SetVerbosityLevel("Language", Verbosity::Warning);
    Log::SetVerbosityLevel("GraphNode", Verbosity::Warning);
    Log::SetVerbosityLevel("Parser", Verbosity::Warning);
    Log::SetVerbosityLevel("VM", Verbosity::Verbose);

    R::init();

    return RUN_ALL_TESTS();
}