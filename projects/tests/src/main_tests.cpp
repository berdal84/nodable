#include <gtest/gtest.h>
#include "fw/core/log.h"
#include <fw/core/reflection/reflection>

using namespace fw;

int main(int argc, char **argv) {

    ::testing::InitGoogleTest(&argc, argv);

    log::set_verbosity(log::Verbosity_Message);

    type_register::log_statistics();

    return RUN_ALL_TESTS();
}