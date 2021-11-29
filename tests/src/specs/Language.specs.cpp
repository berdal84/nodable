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
    const Operator* op = lang->findOperator("+");
    EXPECT_TRUE(op != nullptr);
}

TEST(Language, can_get_add_operator_with_signature )
{
    FunctionSignature signature("operator+", TokenType_DoubleType);
    signature.pushArgs(TokenType_DoubleType, TokenType_DoubleType);
    const Language* lang = LanguageFactory::GetNodable();
    const Operator* op = lang->findOperator((const FunctionSignature*)&signature );
    EXPECT_TRUE(op != nullptr);
}

TEST(Language, can_get_invert_operator_with_signature )
{
    FunctionSignature signature("operator-", TokenType_DoubleType);
    signature.pushArgs(TokenType_DoubleType); // is unary
    const Language* lang = LanguageFactory::GetNodable();
    const Operator* op = lang->findOperator((const FunctionSignature*)&signature );
    EXPECT_TRUE(op != nullptr);
}