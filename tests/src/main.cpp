#include <gtest/gtest.h>
#include <nodable/Log.h>
#include <nodable/Reflect.h>

using namespace Nodable;
using Verbosity = Log::Verbosity;

int main(int argc, char **argv) {

    ::testing::InitGoogleTest(&argc, argv);

    Log::SetVerbosityLevel("Reflect", Verbosity::Warning);
    Log::SetVerbosityLevel("Language", Verbosity::Warning);
    Log::SetVerbosityLevel("GraphNode", Verbosity::Warning);
    Log::SetVerbosityLevel("Parser", Verbosity::Message);
    Log::SetVerbosityLevel("VM", Verbosity::Message);

    Reflect::Initialize();

    return RUN_ALL_TESTS();
}