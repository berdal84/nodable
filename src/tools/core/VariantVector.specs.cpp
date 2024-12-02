#include "VariantVector.h"
#include "tools/core/log.h"
#include <gtest/gtest.h>

using namespace tools;

TEST(Variant, basics )
{
    using MyVariant = Variant<u64_t, u32_t>;
    MyVariant v = u32_t{255};
    EXPECT_TRUE( v.index() == MyVariant::index_of<u32_t>() );

    v = u64_t{255};
    EXPECT_TRUE( v.index() == MyVariant::index_of<u64_t>() );

    v = {};
    EXPECT_TRUE( v.empty() );
    EXPECT_TRUE( v.index() == MyVariant::index_null );
}

TEST(VariantVector, append_contains )
{
    using MyVec = VariantVector<u64_t, const char*, std::string>;
    using Elem = MyVec::element_t;
    MyVec vec;

    Elem a = 1984u;
    Elem b = 2024u;

    vec.append(a);
    vec.append(b);

    EXPECT_TRUE( a != b );
    EXPECT_TRUE( a.hash() != b.hash() );
    EXPECT_TRUE( vec.contains(a) );
    EXPECT_TRUE( vec.contains(b) );

    EXPECT_TRUE( vec.data().front() == a );
    EXPECT_TRUE( vec.data().back() == b );

    vec.clear();

    EXPECT_FALSE( vec.contains(a) );
    EXPECT_FALSE( vec.contains(b) );

    vec.append(a);
    EXPECT_TRUE( vec.contains(a) );
}
