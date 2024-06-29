#include <gtest/gtest.h>

#include "ndbl/core/fixtures/core.h"
#include "tools/core/reflection/func_type.h"

using namespace ndbl;
using namespace tools;

typedef testing::Core Language_basics;

TEST_F(Language_basics, can_get_add_operator_with_short_identifier )
{
    EXPECT_TRUE(language->find_operator("+", Operator_t::Binary));
    EXPECT_TRUE(language->find_operator("-", Operator_t::Unary));
}

TEST_F(Language_basics, can_get_add_operator_with_signature )
{
    const func_type*  signature = func_type_builder<double(double, double)>::with_id("+");
    EXPECT_TRUE(language->find_operator_fct(signature));
}

TEST_F(Language_basics, can_get_invert_operator_with_signature )
{
    const func_type*  signature = func_type_builder<double(double)>::with_id("-");
    EXPECT_TRUE(language->find_operator_fct(signature));
}

TEST_F(Language_basics, by_ref_assign )
{
    const func_type*  signature = func_type_builder<double(double &, double)>::with_id("=");
    auto operator_func = language->find_operator_fct(signature);
    EXPECT_TRUE(operator_func != nullptr);

    // prepare call
    variant left(50.0);
    variant right(200.0);
    variant result(0.0);
    std::vector<variant*> args{&left, &right};

    // call
    result = operator_func->invoke(args);

    //check
    EXPECT_DOUBLE_EQ((double)left, 200.0);
    EXPECT_DOUBLE_EQ((double)result, 200.0);
}

TEST_F(Language_basics, token_t_to_type)
{
    EXPECT_EQ(language->get_type(Token_t::keyword_bool)  , type::get<bool>());
    EXPECT_EQ(language->get_type(Token_t::keyword_double), type::get<double>() );
    EXPECT_EQ(language->get_type(Token_t::keyword_i16)   , type::get<i16_t>() );
    EXPECT_EQ(language->get_type(Token_t::keyword_int)   , type::get<int>() );
    EXPECT_EQ(language->get_type(Token_t::keyword_int)   , type::get<i32_t>() );
    EXPECT_EQ(language->get_type(Token_t::keyword_string), type::get<std::string>() );
}

TEST_F(Language_basics, type_to_string)
{
    EXPECT_EQ(language->to_string(type::get<bool>())        , "bool" );
    EXPECT_EQ(language->to_string(type::get<double>())      , "double" );
    EXPECT_EQ(language->to_string(type::get<i16_t>())       , "i16" );
    EXPECT_EQ(language->to_string(type::get<int>())         , "int" );
    EXPECT_EQ(language->to_string(type::get<i32_t>())       , "int" );
    EXPECT_EQ(language->to_string(type::get<std::string>()) , "string" );
}
