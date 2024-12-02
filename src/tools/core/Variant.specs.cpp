#include "Variant.h"
#include "tools/core/log.h"
#include <gtest/gtest.h>

using namespace tools;

TEST(Variant, basics )
{
    using MyVariant = Variant<u64_t, u32_t>;
    MyVariant v;
    EXPECT_TRUE( v.empty() );
    EXPECT_TRUE( v.index() == MyVariant::index_null );

    v = u32_t{255};
    EXPECT_TRUE( v.index() == MyVariant::index_of<u32_t>() );

    v = u64_t{255};
    EXPECT_TRUE( v.index() == MyVariant::index_of<u64_t>() );

    v = {};
    EXPECT_TRUE( v.empty() );
    EXPECT_TRUE( v.index() == MyVariant::index_null );
}
