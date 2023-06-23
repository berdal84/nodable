#include <gtest/gtest.h>

#include <fw/core/reflection/invokable.h>
#include "../fixtures/core.h"

using namespace ndbl;
using namespace fw;

typedef testing::Core Language;

TEST_F(Language, no_arg_fct)
{
    auto no_arg_fct = func_type_builder<bool()>::with_id("fct");
    EXPECT_EQ(no_arg_fct->get_arg_count(), 0);
}

TEST_F(Language, push_single_arg)
{
    func_type* single_arg_fct = func_type_builder<bool(double)>::with_id("fct");

    EXPECT_EQ(single_arg_fct->get_arg_count(), 1);
    EXPECT_EQ(single_arg_fct->get_return_type(), type::get<bool>());
    EXPECT_EQ(single_arg_fct->get_args().at(0).m_type, type::get<double>());
}

TEST_F(Language, push_two_args)
{
    auto two_arg_fct = func_type_builder<bool(double, double)>::with_id("fct");

    EXPECT_EQ(two_arg_fct->get_arg_count(), 2);
    EXPECT_EQ(two_arg_fct->get_return_type(), type::get<bool>());
    EXPECT_EQ(two_arg_fct->get_args().at(0).m_type, type::get<double>());
    EXPECT_EQ(two_arg_fct->get_args().at(1).m_type, type::get<double>());
}

TEST_F(Language, match_check_for_arg_count)
{
    func_type* single_arg_fct = func_type_builder<bool(bool)>::with_id("fct");
    func_type* two_arg_fct    = func_type_builder<bool(bool, bool)>::with_id("fct");

    EXPECT_EQ(two_arg_fct->is_compatible(single_arg_fct), false);
    EXPECT_EQ(single_arg_fct->is_compatible(two_arg_fct), false);
}

TEST_F(Language, match_check_identifier)
{
    func_type* two_arg_fct          = func_type_builder<bool(bool, bool)>::with_id("fct");
    func_type* two_arg_fct_modified = func_type_builder<bool()>::with_id("fct");

    two_arg_fct_modified->push_arg(type::get<double>() );
    two_arg_fct_modified->push_arg(type::get<double>() );

    EXPECT_EQ(two_arg_fct->is_compatible(two_arg_fct_modified), false);
    EXPECT_EQ(two_arg_fct_modified->is_compatible(two_arg_fct), false);
}

TEST_F(Language, match_check_absence_of_arg)
{
    func_type* two_arg_fct              = func_type_builder<bool(bool, bool)>::with_id("fct");
    func_type* two_arg_fct_without_args = func_type_builder<bool()>::with_id("fct");

    EXPECT_EQ(two_arg_fct->is_compatible(two_arg_fct_without_args), false);
    EXPECT_EQ(two_arg_fct_without_args->is_compatible(two_arg_fct), false);
}

TEST_F(Language, push_args_template_0)
{
    auto ref = func_type_builder<bool()>::with_id("fct");
    auto fct = func_type_builder<bool()>::with_id("fct");

    using Args = std::tuple<>; // create arg tuple
    fct->push_args<Args>(); // push those args to signature

    EXPECT_EQ(ref->is_compatible(fct), true);
    EXPECT_EQ(fct->get_arg_count(), 0);
}

TEST_F(Language, push_args_template_1)
{
    auto ref = func_type_builder<bool(double, double)>::with_id("fct");
    auto fct = func_type_builder<bool()>::with_id("fct");

    fct->push_args< std::tuple<double, double> >();

    EXPECT_EQ(ref->is_compatible(fct), true);
    EXPECT_EQ(fct->get_arg_count(), 2);
}

TEST_F(Language, push_args_template_4)
{
    auto ref = func_type_builder<bool(double, double, double, double)>::with_id("fct");
    auto fct = func_type_builder<bool()>::with_id("fct");
    fct->push_args< std::tuple<double, double, double, double> >();

    EXPECT_EQ(ref->is_compatible(fct), true);
    EXPECT_EQ(fct->get_arg_count(), 4);
}

TEST_F(Language, can_get_add_operator_with_short_identifier )
{
    EXPECT_TRUE(nodlang.find_operator("+", Operator_t::Binary));
    EXPECT_TRUE(nodlang.find_operator("-", Operator_t::Unary));
}

TEST_F(Language, can_get_add_operator_with_signature )
{
    const func_type*  signature = func_type_builder<double(double, double)>::with_id("+");
    EXPECT_TRUE(nodlang.find_operator_fct(signature));
}

TEST_F(Language, can_get_invert_operator_with_signature )
{
    const func_type*  signature = func_type_builder<double(double)>::with_id("-");
    EXPECT_TRUE(nodlang.find_operator_fct(signature));
}

TEST_F(Language, by_ref_assign )
{
    const func_type*  signature = func_type_builder<double(double &, double)>::with_id("=");
    auto operator_func = nodlang.find_operator_fct(signature);
    EXPECT_TRUE(operator_func != nullptr);

    // prepare call
    variant left(50.0);
    variant right(200.0);
    variant result(0.0);
    std::vector<variant*> args{&left, &right};

    // call
    result = (*operator_func)(args);

    //check
    EXPECT_DOUBLE_EQ((double)left, 200.0);
    EXPECT_DOUBLE_EQ((double)result, 200.0);
}

TEST_F(Language, token_t_to_type)
{
    EXPECT_EQ(nodlang.get_type(Token_t::keyword_bool)  , type::get<bool>());
    EXPECT_EQ(nodlang.get_type(Token_t::keyword_double), type::get<double>() );
    EXPECT_EQ(nodlang.get_type(Token_t::keyword_int)   , type::get<i16_t>() );
    EXPECT_EQ(nodlang.get_type(Token_t::keyword_string), type::get<std::string>() );
}

TEST_F(Language, type_to_string)
{
    EXPECT_EQ(nodlang.to_string(type::get<bool>())        , "bool" );
    EXPECT_EQ(nodlang.to_string(type::get<double>())      , "double" );
    EXPECT_EQ(nodlang.to_string(type::get<i16_t>())       , "int" );
    EXPECT_EQ(nodlang.to_string(type::get<std::string>()) , "string" );
}
