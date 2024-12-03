#include "Variant.h"
#include "UniqueVariantList.h"
#include "tools/core/log.h"
#include <gtest/gtest.h>

using namespace tools;

TEST(UniqueVectorList, with_Variant )
{
    using MyVariant  = Variant<u64_t, const char*, std::string>;
    using MyVec      = UniqueVariantList<MyVariant>;

    MyVec vec;

    MyVariant a = 1984u;
    MyVariant b = 2024u;

    vec.append(a);
    vec.append(b);

    std::hash<MyVariant> hasher;

    EXPECT_TRUE( a != b );
    EXPECT_TRUE( hasher(a) != hasher(b) );
    EXPECT_TRUE( vec.contains(a) );
    EXPECT_TRUE( vec.contains(b) );

    EXPECT_TRUE( vec.front() == a );
    EXPECT_TRUE( vec.back() == b );

    vec.clear();

    EXPECT_FALSE( vec.contains(a) );
    EXPECT_FALSE( vec.contains(b) );

    vec.append(a);
    EXPECT_TRUE( vec.contains(a) );
}
