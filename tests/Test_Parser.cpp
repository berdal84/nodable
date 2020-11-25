
#include "gtest/gtest.h"
#include <Core/Member.h>
#include <Component/Container.h>
#include <Language/Parser.h>
#include <Node/Variable.h>

using namespace Nodable;

template <typename T>
bool Parser_Test(
        const std::string& expression,
        T _expectedValue,
        const Language* _language = Language::Nodable()
){

    Container container(_language);
    Parser parser(_language, &container);

    parser.eval(expression);

    auto result = container.getResultVariable();
    container.update();

    auto expectedMember = std::make_unique<Member>(nullptr);
    expectedMember->set(_expectedValue);
    auto success = result->getMember()->equals(expectedMember.get());

    return success;
}


std::string ParseEvalSerialize(
        const std::string& expression,
        const Language* _language = Language::Nodable()
){

    Container container(_language);
    Parser parser(_language, &container);

    parser.eval(expression);

    auto result = container.getResultVariable();
    container.update();

    auto resultExpression = _language->serialize(result->getMember());

    std::cout << resultExpression << std::endl;

    return resultExpression;
}

TEST(Parser, DNAtoProtein )
{
    EXPECT_TRUE(Parser_Test("DNAtoProtein(\"TAA\")", "_"));
    EXPECT_TRUE(Parser_Test("DNAtoProtein(\"TAG\")", "_"));
    EXPECT_TRUE(Parser_Test("DNAtoProtein(\"TGA\")", "_"));
    EXPECT_TRUE(Parser_Test("DNAtoProtein(\"ATG\")", "M"));
}


TEST(Parser, Simple_expressions)
{
    EXPECT_TRUE( Parser_Test("-5", -5));
    EXPECT_TRUE( Parser_Test("2+3", 5));
    EXPECT_TRUE( Parser_Test("-5+4", -1));
    EXPECT_TRUE( Parser_Test("-1+2*5-3/6", 8.5));
}

TEST(Parser, Simple_parenthesis)
{
    EXPECT_TRUE( Parser_Test("(1+4)", 5));
    EXPECT_TRUE( Parser_Test("(1)+(2)", 3));
    EXPECT_TRUE( Parser_Test("(1+2)*3", 9));
    EXPECT_TRUE( Parser_Test("2*(5+3)", 16));
}

TEST(Parser, Unary_operators)
{
    EXPECT_TRUE(Parser_Test("-1*20", -20));
    EXPECT_TRUE(Parser_Test("-(1+4)", -5));
    EXPECT_TRUE(Parser_Test("(-1)+(-2)", -3));
    EXPECT_TRUE(Parser_Test("-5*3", -15));
    EXPECT_TRUE(Parser_Test("2-(5+3)", -6));
}

TEST(Parser, Complex_parenthesis)
{
    EXPECT_TRUE(Parser_Test("2+(5*3)", 2+(5*3)));
    EXPECT_TRUE(Parser_Test("2*(5+3)+2", 2*(5+3)+2));
    EXPECT_TRUE(Parser_Test("(2-(5+3))-2+(1+1)", (2-(5+3))-2+(1+1)));
    EXPECT_TRUE(Parser_Test("(2 -(5+3 )-2)+9/(1- 0.54)", (2 -(5+3 )-2)+9/(1- 0.54)));
    EXPECT_TRUE(Parser_Test("1/3", 1.0F/3.0F));
}

TEST(Parser, Function_call)
{
    EXPECT_TRUE(Parser_Test("returnNumber(5)", 5));
    EXPECT_TRUE(Parser_Test("returnNumber(1)", 1));
    EXPECT_TRUE(Parser_Test("sqrt(81)", 9));
    EXPECT_TRUE(Parser_Test("pow(2,2)", 4));
}

TEST(Parser, FunctionLike_operators_call)
{
    EXPECT_TRUE(Parser_Test("operator*(2,2)", double(4)));
    EXPECT_TRUE(Parser_Test("operator>(2,2)", false));
    EXPECT_TRUE(Parser_Test("operator-(3,2)", double(1)));
    EXPECT_TRUE(Parser_Test("operator+(2,2)", double(4)));
    EXPECT_TRUE(Parser_Test("operator/(4,2)", double(2)));
}

TEST(Parser, Imbricated_functions)
{
    EXPECT_TRUE(Parser_Test("returnNumber(5+3)", 8));
    EXPECT_TRUE(Parser_Test("returnNumber(returnNumber(1))", 1));
    EXPECT_TRUE(Parser_Test("returnNumber(returnNumber(1) + returnNumber(1))", 2));
}

TEST(Parser, Successive_assigns)
{
    EXPECT_TRUE(Parser_Test("a = b = 5", 5));
    EXPECT_TRUE(Parser_Test("a = b = c = 10", 10));
}

TEST(Parser, Strings)
{
    EXPECT_TRUE(Parser_Test("a = \"coucou\"", "coucou"));
    EXPECT_TRUE(Parser_Test("\"hello \" + \"world\"", "hello world"));
    EXPECT_TRUE(Parser_Test("a = string(15)", "15"));
    EXPECT_TRUE(Parser_Test("a = string(-15)", "-15"));
    EXPECT_TRUE(Parser_Test("a = string(-15.5)", "-15.5"));
    EXPECT_TRUE(Parser_Test("b = string(true)", "true"));
    EXPECT_TRUE(Parser_Test("b = string(false)", "false"));
}

TEST(Parser, Serialize_Precedence)
{
    EXPECT_EQ(ParseEvalSerialize("(1+1)*2"), "(1 + 1) * 2");
    EXPECT_EQ(ParseEvalSerialize("(1*1)+2"), "1 * 1 + 2");
    EXPECT_EQ(ParseEvalSerialize("-(-1)"), "-(-1)");
    EXPECT_EQ(ParseEvalSerialize("-(-1)"), "-(-1)");
    EXPECT_EQ(ParseEvalSerialize("-(2*5)"), "-(2 * 5)");
    EXPECT_EQ(ParseEvalSerialize("-2*5"), "(-2) * 5");
    EXPECT_EQ(ParseEvalSerialize("-(2+5)"), "-(2 + 5)");
    EXPECT_EQ(ParseEvalSerialize("5 + (-1) * 3"), "5 + (-1) * 3");
}

TEST(Parser, Eval_Serialize_Compare)
{
    // TODO: problem with result variable not updating its source expression when expression is atomic
    EXPECT_EQ(ParseEvalSerialize("1"), "1");
    EXPECT_EQ(ParseEvalSerialize("1+1"), "1 + 1");
    EXPECT_EQ(ParseEvalSerialize("1-1"), "1 - 1");
    EXPECT_EQ(ParseEvalSerialize("-1"), "-1");
    EXPECT_EQ(ParseEvalSerialize("a=5"), "a = 5");
    EXPECT_EQ(ParseEvalSerialize("(a+b)*(c+d)"), "(a + b) * (c + d)");
    EXPECT_EQ(ParseEvalSerialize("b = string(false)"), "b = string(false)");
}