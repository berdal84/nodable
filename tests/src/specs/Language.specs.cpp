#include <gtest/gtest.h>

#include <nodable/LanguageFactory.h>

using namespace Nodable;

TEST(Language, can_get_nodable_language )
{
    const Language* lang = LanguageFactory::GetNodable();
    EXPECT_TRUE(lang != nullptr);
}

TEST(Language, can_get_add_operator_with_short_identifier )
{
    const Language* lang = LanguageFactory::GetNodable();
    const InvokableOperator* op = lang->findOperator("+");
    EXPECT_TRUE(op != nullptr);
}

TEST(Language, can_get_add_operator_with_signature )
{
    FunctionSignature signature = FunctionSignature::create(Type_Double, "operator+", Type_Double, Type_Double);
    const Language* lang = LanguageFactory::GetNodable();
    const InvokableOperator* op = lang->findOperator((const FunctionSignature*)&signature );
    EXPECT_TRUE(op != nullptr);
}

TEST(Language, can_get_invert_operator_with_signature )
{
    FunctionSignature signature = FunctionSignature::create(Type_Double, "operator-", Type_Double);
    const Language* lang = LanguageFactory::GetNodable();
    const InvokableOperator* op = lang->findOperator((const FunctionSignature*)&signature );
    EXPECT_TRUE(op != nullptr);
}