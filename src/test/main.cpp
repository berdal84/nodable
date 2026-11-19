#include <gtest/gtest.h>
#include "bdc/Signal_Handler.hpp"

int main(int argc, char **argv)
{
    BDC_INSTALL_SIGNAL_HANDLER();
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}