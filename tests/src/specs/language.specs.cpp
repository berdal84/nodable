#include <gtest/gtest.h>

#include <nodable/core/Invokable.h>
#include <nodable/core/SignatureBuilder.h>
#include <nodable/core/languages/NodableLanguage.h>

using namespace Nodable;

class language_fixture: public ::testing::Test
{
public:
    NodableLanguage  language; // signatures must be created through a language.
};

TEST_F(language_fixture, no_arg_fct)
{
    auto no_arg_fct = SignatureBuilder<bool()>::new_function("fct", &language);

    EXPECT_EQ(no_arg_fct->get_arg_count(), 0);
    EXPECT_EQ(no_arg_fct->get_arg_count(), 0);
}

TEST_F(language_fixture, push_single_arg)
{
    Signature* single_arg_fct = SignatureBuilder<bool(double)>::new_function("fct", &language);

    EXPECT_EQ(single_arg_fct->get_arg_count(), 1);
    EXPECT_EQ(single_arg_fct->get_return_type(), type::get<bool>());
    EXPECT_EQ(single_arg_fct->get_args().at(0).m_type, type::get<double>());
}

TEST_F(language_fixture, push_two_args)
{
    auto two_arg_fct = SignatureBuilder<bool(double, double)>::new_function("fct", &language);

    EXPECT_EQ(two_arg_fct->get_arg_count(), 2);
    EXPECT_EQ(two_arg_fct->get_return_type(), type::get<bool>());
    EXPECT_EQ(two_arg_fct->get_args().at(0).m_type, type::get<double>());
    EXPECT_EQ(two_arg_fct->get_args().at(1).m_type, type::get<double>());
}

TEST_F(language_fixture, match_check_for_arg_count)
{
    Signature* single_arg_fct = SignatureBuilder<bool(bool)>::new_function("fct", &language);
    Signature* two_arg_fct    = SignatureBuilder<bool(bool, bool)>::new_function("fct", &language);

    EXPECT_EQ(two_arg_fct->is_compatible(single_arg_fct), false);
    EXPECT_EQ(single_arg_fct->is_compatible(two_arg_fct), false);
}

TEST_F(language_fixture, match_check_identifier)
{
    Signature* two_arg_fct          = SignatureBuilder<bool(bool, bool)>::new_function("fct", &language);
    Signature* two_arg_fct_modified = SignatureBuilder<bool()>::new_function("fct", &language);

    two_arg_fct_modified->push_arg(type::get<double>() );
    two_arg_fct_modified->push_arg(type::get<double>() );

    EXPECT_EQ(two_arg_fct->is_compatible(two_arg_fct_modified), false);
    EXPECT_EQ(two_arg_fct_modified->is_compatible(two_arg_fct), false);
}

TEST_F(language_fixture, match_check_absence_of_arg)
{
    Signature* two_arg_fct              = SignatureBuilder<bool(bool, bool)>::new_function("fct", &language);
    Signature* two_arg_fct_without_args = SignatureBuilder<bool()>::new_function("fct", &language);

    EXPECT_EQ(two_arg_fct->is_compatible(two_arg_fct_without_args), false);
    EXPECT_EQ(two_arg_fct_without_args->is_compatible(two_arg_fct), false);
}

TEST_F(language_fixture, push_args_template_0)
{
    auto ref = SignatureBuilder<bool()>::new_function("fct", &language);
    auto fct = SignatureBuilder<bool()>::new_function("fct", &language);

    using Args = std::tuple<>; // create arg tuple
    fct->push_args<Args>(); // push those args to signature

    EXPECT_EQ(ref->is_compatible(fct), true);
    EXPECT_EQ(fct->get_arg_count(), 0);
}

TEST_F(language_fixture, push_args_template_1)
{
    auto ref = SignatureBuilder<bool(double, double)>::new_function("fct", &language);
    auto fct = SignatureBuilder<bool()>::new_function("fct", &language);

    fct->push_args< std::tuple<double, double> >();

    EXPECT_EQ(ref->is_compatible(fct), true);
    EXPECT_EQ(fct->get_arg_count(), 2);
}

TEST_F(language_fixture, push_args_template_4)
{
    auto ref = SignatureBuilder<bool(double, double, double, double)>::new_function("fct", &language);
    auto fct = SignatureBuilder<bool()>::new_function("fct", &language);
    fct->push_args< std::tuple<double, double, double, double> >();

    EXPECT_EQ(ref->is_compatible(fct), true);
    EXPECT_EQ(fct->get_arg_count(), 4);
}

TEST_F(language_fixture, sanitize_function_id)
{
    {
        auto sig = SignatureBuilder<bool(double, double, double, double)>::new_function("api_fct", &language);
        EXPECT_EQ(sig->get_identifier(), "fct");
    }

    {
        auto sig = SignatureBuilder<bool(double, double, double, double)>::new_function("fct", &language);
        EXPECT_EQ(sig->get_identifier(), "fct");
    }

    {
        EXPECT_ANY_THROW( SignatureBuilder<bool(double, double, double, double)>::new_function("api_", &language) );
    }
}

TEST_F(language_fixture, sanitize_operator_function_id)
{
    {
        auto sig = SignatureBuilder<double(double, double)>::new_operator("+", &language);
        EXPECT_EQ(sig->get_identifier(), std::string{NodableLanguage::k_keyword_operator} + "+");
    }

    {
        auto sig = SignatureBuilder<double(double, double)>::new_operator("==", &language);
        EXPECT_EQ(sig->get_identifier(), std::string{NodableLanguage::k_keyword_operator} + "==");
    }
}

TEST_F(language_fixture, can_get_add_operator_with_short_identifier )
{
    EXPECT_TRUE(language.find_operator("+", Operator_t::Binary));
    EXPECT_TRUE(language.find_operator("-", Operator_t::Unary));
}

TEST_F(language_fixture, can_get_add_operator_with_signature )
{
    const Signature*  signature = SignatureBuilder<double(double, double)>::new_operator("+", &language);
    const IInvokable* operator_ = language.find_operator_fct(signature);
    EXPECT_TRUE(operator_);
}

TEST_F(language_fixture, can_get_invert_operator_with_signature )
{
    const Signature*  signature = SignatureBuilder<double(double)>::new_operator("-", &language);
    const IInvokable* operator_ = language.find_operator_fct(signature);
    EXPECT_TRUE(operator_);
}

TEST_F(language_fixture, by_ref_assign )
{
    const Signature*  signature = SignatureBuilder<double(double&,double)>::new_operator("=", &language);
    const IInvokable* operator_ = language.find_operator_fct(signature);
    EXPECT_TRUE(operator_ != nullptr);

    // prepare call
    Member left(nullptr, 50.0);
    Member right(nullptr, 200.0);
    Member result(nullptr, 0.0);
    std::vector<Member*> args{&left, &right};

    // call
    operator_->invoke(&result, args);

    //check
    EXPECT_DOUBLE_EQ((double)left, 200.0);
    EXPECT_DOUBLE_EQ((double)result, 200.0);
}

TEST_F(language_fixture, token_t_to_type)
{
    EXPECT_EQ(language.get_type(Token_t::keyword_bool)  , type::get<bool>());
    EXPECT_EQ(language.get_type(Token_t::keyword_double), type::get<double>() );
    EXPECT_EQ(language.get_type(Token_t::keyword_int)   , type::get<i16_t>() );
    EXPECT_EQ(language.get_type(Token_t::keyword_string), type::get<std::string>() );
}

TEST_F(language_fixture, type_to_string)
{
    EXPECT_EQ(language.to_string(type::get<bool>())        , NodableLanguage::k_keyword_bool );
    EXPECT_EQ(language.to_string(type::get<double>())      , NodableLanguage::k_keyword_double  );
    EXPECT_EQ(language.to_string(type::get<i16_t>())       , NodableLanguage::k_keyword_int  );
    EXPECT_EQ(language.to_string(type::get<std::string>()) , NodableLanguage::k_keyword_string  );
}
