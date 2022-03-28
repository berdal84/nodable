#include <gtest/gtest.h>
#include <nodable/core/languages/Nodable.h>

using namespace Nodable;

TEST(Language, can_get_add_operator_with_short_identifier )
{
    LanguageNodable language;
    const InvokableOperator* op = language.findOperator("+");
    EXPECT_TRUE(op != nullptr);
}

TEST(Language, can_get_add_operator_with_signature )
{
    LanguageNodable language;
    const FunctionSignature* signature = FunctionSignature::new_instance<double(double, double)>::with_id("operator+");
    const InvokableOperator* op = language.findOperator(signature);
    EXPECT_TRUE(op != nullptr);
}

TEST(Language, can_get_invert_operator_with_signature )
{
    LanguageNodable language;
    const FunctionSignature* signature = FunctionSignature::new_instance<double(double)>::with_id("operator-");
    const InvokableOperator* op = language.findOperator(signature);
    EXPECT_TRUE(op != nullptr);
}

TEST(Language, by_pointer_assign )
{
    LanguageNodable language;
    // find double operator=(double*, double)
    const FunctionSignature* signature = FunctionSignature::new_instance<double(double*, double)>::with_id("operator=");
    const InvokableOperator* op = language.findOperator(signature);
    EXPECT_TRUE(op != nullptr);

    // prepare call
    Member left(nullptr, 50.0);
    Member right(nullptr, 200.0);
    Member result(nullptr, 0.0);
    std::vector<Member*> args{&left, &right};

    // call
    op->invoke(&result, args);

    //check
    EXPECT_DOUBLE_EQ((double)left, 200.0);
    EXPECT_DOUBLE_EQ((double)result, 200.0);
}