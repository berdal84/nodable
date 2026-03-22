#include "tools/gui/unity_build.cpp"

#include "tools/core/Delegate.specs.cpp"
#include "tools/core/Containers.specs.cpp"
#include "tools/core/string.specs.cpp"
#include "tools/core/reflection/reflection.specs.cpp"

#include "tools/gui/geometry/SpatialNode.specs.cpp"
#include "tools/gui/geometry/Rect.specs.cpp"

#include "tools/core/log.h"
#include "tools/core/reflection/reflection"
#include <gtest/gtest.h>

int main(int argc, char **argv)
{
    tools::init_reflection();
    tools::set_log_verbosity(tools::Verbosity_Message);
    tools::TypeRegister::log_statistics();

    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}