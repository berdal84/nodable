#include "ndbl/gui/index.h"

#include <gtest/gtest.h>

int main(int argc, char **argv)
{
    ndbl::init_with_gui();
    tools::set_log_verbosity(tools::Verbosity_Message);
    tools::type_register_log_statistics();

    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}