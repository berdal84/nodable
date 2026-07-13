#include <gtest/gtest.h>
#include "tools/core/String.h"

using namespace tools;

TEST(String, constructor_no_args)
{
    String str;
    EXPECT_STREQ(str.c_str(), "");
    EXPECT_EQ(str.size, 0);
    EXPECT_EQ(str.capacity, 0);
}

TEST(String_16, constructor_no_args)
{
    String_16 str;
    EXPECT_STREQ(str.c_str(), "");
    EXPECT_EQ(str.size, 0);
    EXPECT_EQ(str.capacity, 15);
}

TEST(String_16, constructor_with_args)
{
    String_16 str("Super");

    EXPECT_STREQ(str.c_str(), "Super");
    EXPECT_EQ(str.size, 5);
    EXPECT_EQ(str.capacity, 15);
}

TEST(String16, append_char)
{
    String_16 str("Super");

    EXPECT_STREQ(str.c_str(), "Super");
    EXPECT_EQ(str.size, 5);

    string_push_back(&str, '!');

    EXPECT_EQ(str.size, 6);
}

TEST(String_16, append_strn)
{
    String_16 str("Super");

    EXPECT_STREQ(str.c_str(), "Super");
    EXPECT_STREQ(str.c_str(), "Super");
    EXPECT_EQ(str.size, 5);

    string_append(&str, "man!", 4);

    EXPECT_EQ(str.size, 9);
}

TEST(String_16, append_const_char_ptr)
{
    String_16 str("Super");

    EXPECT_STREQ(str.c_str(), "Super");
    EXPECT_EQ(str.size, 5);

    string_append(&str, "man!");

    EXPECT_EQ(str.size, 9);
}

TEST(String_8, append_overflow)
{
    String_8 str("Super");

    EXPECT_FALSE(str.heap_allocated());

    string_append(&str, " long");

    EXPECT_STREQ(str.c_str(), "Super long");
    EXPECT_TRUE(str.heap_allocated());
}

TEST(String, append_overflow)
{
    String str("Super");

    EXPECT_TRUE(str.heap_allocated());
    EXPECT_EQ(str.capacity, 8-1);

    string_append(&str, " long");

    EXPECT_STREQ(str.c_str(), "Super long");
    EXPECT_EQ(str.capacity, 16-1);

    string_append(&str, ", mais vraiment beaucoup ça race");

    EXPECT_STREQ(str.c_str(), "Super long, mais vraiment beaucoup ça race");
    EXPECT_EQ(str.capacity, 64-1);
}

TEST(String_8, copy )
{
    String_8 str("Super");
    String_8 copy = str;
    EXPECT_STREQ(str.c_str(), copy.c_str());
    string_append(&str, " bien!");
    EXPECT_STRNE(str.c_str(), copy.c_str());
}

TEST(String_8, copy_constructor )
{
    String_8 str("Super");
    String_8 copy(str);
    EXPECT_STREQ(str.c_str(), copy.c_str());
    string_append(&str, " bien!");
    EXPECT_STRNE(str.c_str(), copy.c_str());
}

TEST(String, String_copy )
{
    String str("Super");
    String copy = str;
    EXPECT_STREQ(str.c_str(), copy.c_str());
    string_append(&str, " bien!");
    EXPECT_STRNE(str.c_str(), copy.c_str());
}

TEST(String, copy_String_8 )
{
    String_8 str("Super");
    String copy = str;
    EXPECT_STREQ(str.c_str(), copy.c_str());
    string_append(&str, " bien!");
    EXPECT_STRNE(str.c_str(), copy.c_str());
}

TEST(String_8, copy_String )
{
    String str("Super");
    String_8 copy = str;
    EXPECT_STREQ(str.c_str(), copy.c_str());
    EXPECT_FALSE(copy.heap_allocated());
}

TEST(String_8, move_ctor )
{
    String str("My string is very long");
    String copy(std::move(str));
    EXPECT_STREQ(copy.c_str(), "My string is very long");
    EXPECT_STREQ(str.c_str(), "");
}

TEST(String_8, move_assignment)
{
    String str("My string is very long");
    String copy = std::move(str);
    EXPECT_STREQ(copy.c_str(), "My string is very long");
    EXPECT_STREQ(str.c_str(), "");
}