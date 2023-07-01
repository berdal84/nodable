#include <gtest/gtest.h>
#include <fw/core/string.h>

/*

 TODO: implement an Str<0> optimized to only use dynamic allocations
TEST(fw_Str, heap_allocated_constructor_no_args)
{
    fw::Str str;
    EXPECT_STREQ(str.c_str(), "");
    EXPECT_EQ(str.length(), 0);
    EXPECT_EQ(str.is_empty(), true);
    EXPECT_EQ(str.capacity(), 1);
}
*/

TEST(fw_Str, stack_allocated_constructor_no_args)
{
    fw::Str16 str;
    EXPECT_STREQ(str.c_str(), "");
    EXPECT_EQ(str.length(), 0);
    EXPECT_EQ(str.is_empty(), true);
    EXPECT_EQ(str.capacity(), 15);
}

TEST(fw_Str, stack_allocated_constructor_with_args)
{
    fw::Str16 str("Super");
    EXPECT_STREQ(str.c_str(), "Super");
    EXPECT_EQ(str.length(), 5);
    EXPECT_EQ(str.is_empty(), false);
    EXPECT_EQ(str.capacity(), 15);
}

TEST(fw_Str, append_char)
{
    fw::Str16 str("Super");
    EXPECT_STREQ(str.c_str(), "Super");
    EXPECT_EQ(str.length(), 5);
    str.append('!');
    EXPECT_EQ(str.length(), 6);
}

TEST(fw_Str, append_strn)
{
    fw::Str16 str("Super");
    EXPECT_STREQ(str.c_str(), "Super");
    EXPECT_EQ(str.length(), 5);
    str.append("man!", 4);
    EXPECT_EQ(str.length(), 9);
}

TEST(fw_Str, append_const_char_ptr)
{
    fw::Str16 str("Super");
    EXPECT_STREQ(str.c_str(), "Super");
    EXPECT_EQ(str.length(), 5);
    str.append("man!");
    EXPECT_EQ(str.length(), 9);
}

TEST(fw_Str, append_overflow)
{
    fw::Str8 str("Super");
    EXPECT_FALSE(str.is_on_heap());
    str.append(" long");
    EXPECT_STREQ(str.c_str(), "Super long");
    EXPECT_TRUE(str.is_on_heap());
}