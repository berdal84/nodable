#include <gtest/gtest.h>
#include "log.h"
#include "reflection/reflection"

using namespace tools;

int main(int argc, char **argv) {

    ::testing::InitGoogleTest(&argc, argv);

    log::set_verbosity(log::Verbosity_Message);

    type_register::log_statistics();

    return RUN_ALL_TESTS();
}