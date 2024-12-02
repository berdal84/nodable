#include "UniqueOrderedList.h"
#include "tools/core/log.h"
#include <gtest/gtest.h>

using namespace tools;

TEST(UniqueOrderedList, is_constructible )
{
    using MyVec = UniqueOrderedList<u64_t>;
    EXPECT_TRUE( std::is_constructible_v<MyVec> );
}

