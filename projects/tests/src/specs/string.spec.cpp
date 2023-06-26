#include <gtest/gtest.h>
#include <fw/core/string.h>

TEST(fw_Str, heap_allocated_constructor_no_args)
{
    fw::Str str;
    EXPECT_STREQ(str.c_str(), "");
    EXPECT_EQ(str.length(), 0);
    EXPECT_EQ(str.is_empty(), true);
    EXPECT_EQ(str.capacity(), 1);
}

TEST(fw_Str, stack_allocated_constructor_no_args)
{
    fw::Str<16> str;
    EXPECT_STREQ(str.c_str(), "");
    EXPECT_EQ(str.length(), 0);
    EXPECT_EQ(str.is_empty(), true);
    EXPECT_EQ(str.capacity(), 16);
}

TEST(fw_Str, stack_allocated_constructor_with_args)
{
    fw::Str<16> str("Super");
    EXPECT_STREQ(str.c_str(), "Super");
    EXPECT_EQ(str.length(), 5);
    EXPECT_EQ(str.is_empty(), false);
    EXPECT_EQ(str.capacity(), 16);
}