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

TEST(fw_Str, string16_constructor_no_args)
{
    fw::string16 str;
    EXPECT_STREQ(str.c_str(), "");
    EXPECT_EQ(str.length(), 0);
    EXPECT_EQ(str.is_empty(), true);
    EXPECT_EQ(str.capacity(), 15);
}

TEST(fw_Str, string16_constructor_with_args)
{
    fw::string16 str("Super");

    EXPECT_STREQ(str.c_str(), "Super");
    EXPECT_EQ(str.length(), 5);
    EXPECT_EQ(str.is_empty(), false);
    EXPECT_EQ(str.capacity(), 15);
}

TEST(fw_Str, string16_append_char)
{
    fw::string16 str("Super");

    EXPECT_STREQ(str.c_str(), "Super");
    EXPECT_EQ(str.length(), 5);

    str.append('!');

    EXPECT_EQ(str.length(), 6);
}

TEST(fw_Str, string16_append_strn)
{
    fw::string16 str("Super");

    EXPECT_STREQ(str.c_str(), "Super");
    EXPECT_EQ(str.length(), 5);

    str.append("man!", 4);

    EXPECT_EQ(str.length(), 9);
}

TEST(fw_Str, string16_append_const_char_ptr)
{
    fw::string16 str("Super");

    EXPECT_STREQ(str.c_str(), "Super");
    EXPECT_EQ(str.length(), 5);

    str.append("man!");

    EXPECT_EQ(str.length(), 9);
}

TEST(fw_Str, string8_append_overflow)
{
    fw::string8 str("Super");

    EXPECT_FALSE(str.heap_allocated());

    str.append(" long");

    EXPECT_STREQ(str.c_str(), "Super long");
    EXPECT_TRUE(str.heap_allocated());
}

TEST(fw_Str, string_append_overflow)
{
    fw::string str("Super");

    EXPECT_TRUE(str.heap_allocated());
    EXPECT_EQ(str.capacity(), 8-1);

    str.append(" long");

    EXPECT_STREQ(str.c_str(), "Super long");
    EXPECT_EQ(str.capacity(), 16-1);

    str.append(", mais vraiment beaucoup ça race");

    EXPECT_STREQ(str.c_str(), "Super long, mais vraiment beaucoup ça race");
    EXPECT_EQ(str.capacity(), 64-1);
}