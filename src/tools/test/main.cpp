#include "tools/gui/index.h"

#include <gtest/gtest.h>

int main(int argc, char **argv)
{
    tools::init_reflection();
    tools::set_log_verbosity(tools::Verbosity_Message);
    tools::TypeRegister::log_statistics();

    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}