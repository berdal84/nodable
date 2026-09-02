#include <gtest/gtest.h>

#include "ndbl/core/fixtures/core.h"
#include "tools/core/reflection/index.h"

using namespace ndbl;
using namespace tools;

typedef testing::Core Language_basics;

TEST_F(Language_basics, can_get_add_operator_with_short_identifier )
{
    EXPECT_TRUE(lang_find_operator(language(), {"+", Operator_Type::Binary}));
    EXPECT_TRUE(lang_find_operator(language(), {"-", Operator_Type::Unary}));
}
TEST_F(Language_basics, token_t_to_type)
{
    EXPECT_EQ(lang_get_type(language(), Token_Type_keyword_bool)  , type_get<bool>());
    EXPECT_EQ(lang_get_type(language(), Token_Type_keyword_double), type_get<double>() );
    EXPECT_EQ(lang_get_type(language(), Token_Type_keyword_i16)   , type_get<i16_t>() );
    EXPECT_EQ(lang_get_type(language(), Token_Type_keyword_int)   , type_get<i32_t>() );
    EXPECT_EQ(lang_get_type(language(), Token_Type_keyword_string), type_get<bdc::String>() );
    EXPECT_EQ(lang_get_type(language(), Token_Type_keyword_any)   , type_get<any>() );

    EXPECT_EQ(lang_get_type(language(), Token_Type_literal_bool)    , nullptr);
    EXPECT_EQ(lang_get_type(language(), Token_Type_literal_double)  , nullptr);
    EXPECT_EQ(lang_get_type(language(), Token_Type_literal_int)     , nullptr);
    EXPECT_EQ(lang_get_type(language(), Token_Type_literal_string)  , nullptr);
    EXPECT_EQ(lang_get_type(language(), Token_Type_literal_any)     , nullptr);
}

TEST_F(Language_basics, type_to_string)
{
    EXPECT_EQ(lang_serialize_type(language(), type_get<bool>())        , "bool" );
    EXPECT_EQ(lang_serialize_type(language(), type_get<double>())      , "double" );
    EXPECT_EQ(lang_serialize_type(language(), type_get<i16_t>())       , "i16" );
    EXPECT_EQ(lang_serialize_type(language(), type_get<int>())         , "int" );
    EXPECT_EQ(lang_serialize_type(language(), type_get<i32_t>())       , "int" );
    EXPECT_EQ(lang_serialize_type(language(), type_get<bdc::String>()) , "string" );
    EXPECT_EQ(lang_serialize_type(language(), type_get<any>())         , "any" );
}
