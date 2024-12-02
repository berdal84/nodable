#include "UniqueList.h"
#include <gtest/gtest.h>

using namespace tools;

TEST(UniqueList, is_constructible )
{
    EXPECT_TRUE( std::is_constructible_v<UniqueList<u64_t>> );
}

