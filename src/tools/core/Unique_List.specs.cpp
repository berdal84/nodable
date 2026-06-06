#include "Unique_List.h"
#include <gtest/gtest.h>

using namespace tools;

TEST(Unique_List, is_constructible )
{
    EXPECT_TRUE( std::is_constructible_v<Unique_List<u64_t>> );
}

