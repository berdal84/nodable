#include "Containers.h"
#include "tools/core/log.h"
#include <gtest/gtest.h>

using namespace tools;

TEST(InlineVector, constructor )
{
    InlineVector<u32_t, 16> vec;
    EXPECT_EQ(vec.size, 0);
    EXPECT_EQ(vec.capacity(), 16);
}

TEST(InlineVector, push_back )
{
    InlineVector<u32_t, 16> vec;
    vec.push_back(42);
    EXPECT_EQ(vec.size, 1);
    EXPECT_EQ(vec[0], 42);
}

TEST(InlineVector, erase )
{
    {
        // without a single item
        InlineVector<u32_t, 16> vec;
        vec.push_back(1);
        auto it = vec.erase(vec.begin());
        EXPECT_EQ(it, vec.end());
        EXPECT_EQ(vec.size, 0);
    }

    {
        // without trailing items
        InlineVector<u32_t, 16> vec;
        vec.push_back(1);
        vec.push_back(2);
        vec.push_back(3);
        auto it = vec.erase(vec.end() - 1);
        EXPECT_EQ(it, vec.end());
        EXPECT_EQ(vec.size, 2);
    }

    {
        // with trailing items
        InlineVector<u32_t, 16> vec;           // prepare
        vec.push_back(1);
        vec.push_back(2);
        vec.push_back(3);
        EXPECT_EQ(vec[1], 2);
        auto it = vec.erase(vec.begin() + 1);  // act
        EXPECT_EQ(it, vec.begin()+1);          // check
        EXPECT_EQ(vec.size, 2);
        EXPECT_EQ(vec[1], 3);
    }
}

TEST(InlineVector, for_loop_auto )
{
    InlineVector<u32_t, 16> vec;
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);

    printf("[");
    size_t i = {};
    for(auto it : vec)
    {
        if( i > 0 )
            printf(", ");
        printf("%u\n", it);
    }
    printf("]\n");
}

TEST(InlineVector, for_loop_with_iterators )
{
    InlineVector<u32_t, 16> vec;
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);

    printf("[");
    for(auto it = vec.begin(); it != vec.end(); it++)
    {
        if( it != vec.begin() )
            printf(", ");
        printf("%u", *it);
    }
    printf("]\n");
}
