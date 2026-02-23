#include "tools/core/log.h"
#include "tools/core/reflection/reflection"
#include <gtest/gtest.h>

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    tools::set_log_verbosity(tools::Verbosity_Message);

    tools::TypeRegister::log_statistics();

    return RUN_ALL_TESTS();
}