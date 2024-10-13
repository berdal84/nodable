#include <gtest/gtest.h>

#include "ndbl/core/fixtures/core.h"
#include "tools/core/reflection/reflection"

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
    const FunctionDescriptor descriptor = FunctionDescriptor::create<double(double, double)>("+");
    EXPECT_TRUE(language->find_operator_fct(&descriptor));
}

TEST_F(Language_basics, can_get_invert_operator_with_signature )
{
    const FunctionDescriptor descriptor = FunctionDescriptor::create<double(double)>("-");
    EXPECT_TRUE(language->find_operator_fct(&descriptor));
}

TEST_F(Language_basics, by_ref_assign )
{
    const FunctionDescriptor descriptor = FunctionDescriptor::create<double(double&, double)>("=");

    EXPECT_TRUE(descriptor.get_args()[0].m_by_reference);

    const IInvokable* invokable  = language->find_operator_fct(&descriptor);

    EXPECT_TRUE(invokable != nullptr);
    EXPECT_TRUE(invokable->get_sig()->get_args()[0].m_by_reference);
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
