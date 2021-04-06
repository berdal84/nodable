#include <VirtualMachine.h>
#include "gtest/gtest.h"

#include "Core/Member.h"
#include "Node/GraphNode.h"
#include "Language/Common/Parser.h"
#include "Language/Common/LanguageLibrary.h"
#include "Node/VariableNode.h"
#include "Node/ScopedCodeBlockNode.h"
#include "Node/InstructionNode.h"
#include "Node/ProgramNode.h"

using namespace Nodable;

template <typename T>
bool Parser_Test(
        const std::string& expression,
        T _expectedValue,
        const Language* _language = LanguageLibrary::GetNodable()
){
    // prepare
    bool success = false;
    auto graph = std::make_unique<GraphNode>(_language);

    // act
    _language->getParser()->expressionToGraph(expression, graph.get());

    auto expectedMember = new Member(_expectedValue);

    if ( auto program = graph->getProgram())
    {
        // run
        VirtualMachine vm;
        vm.load(graph->getProgram());
        vm.run();

        // compare result
        auto lastInstruction = program->getLastInstruction();
        if ( lastInstruction )
        {
            auto result = lastInstruction->getValue();
            success = result->equals(expectedMember);
        }
    } else {
        success = ((std::string)*expectedMember).empty();
    }

    delete expectedMember;
    return success;
}


std::string ParseUpdateSerialize(
        const std::string& expression,
        const Language* _language = LanguageLibrary::GetNodable()
){

    GraphNode container(_language);
    Parser* parser = _language->getParser();
    parser->expressionToGraph(expression, &container);

    container.update();

    Serializer* serializer = _language->getSerializer();

    auto resultExpression = serializer->serialize(container.getProgram());

    std::cout << resultExpression << std::endl;

    return resultExpression;
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
//    EXPECT_TRUE(Parser_Test("\"hello \" + \"world\"", "hello world"));
//    EXPECT_TRUE(Parser_Test("a = string(15)", "15"));
//    EXPECT_TRUE(Parser_Test("a = string(-15)", "-15"));
//    EXPECT_TRUE(Parser_Test("a = string(-15.5)", "-15.5"));
//    EXPECT_TRUE(Parser_Test("b = string(true)", "true"));
//    EXPECT_TRUE(Parser_Test("b = string(false)", "false"));
}

TEST(Parser, Serialize_Precedence)
{
    EXPECT_EQ(ParseUpdateSerialize("(1+1)*2"), "(1+1)*2");
    EXPECT_EQ(ParseUpdateSerialize("(1*1)+2"), "1*1+2");
    EXPECT_EQ(ParseUpdateSerialize("-(-1)"), "-(-1)");
    EXPECT_EQ(ParseUpdateSerialize("-(-1)"), "-(-1)");
    EXPECT_EQ(ParseUpdateSerialize("-(2*5)"), "-(2*5)");
    EXPECT_EQ(ParseUpdateSerialize("-2*5"), "(-2)*5");
    EXPECT_EQ(ParseUpdateSerialize("-(2+5)"), "-(2+5)");
    EXPECT_EQ(ParseUpdateSerialize("5+(-1)*3"), "5+(-1)*3");
}

TEST(Parser, Eval_Serialize_Compare)
{
    EXPECT_EQ(ParseUpdateSerialize("1"), "1");
    EXPECT_EQ(ParseUpdateSerialize("1+1"), "1+1");
    EXPECT_EQ(ParseUpdateSerialize("1-1"), "1-1");
    EXPECT_EQ(ParseUpdateSerialize("-1"), "-1");
    EXPECT_EQ(ParseUpdateSerialize("a=5"), "a=5");
    EXPECT_EQ(ParseUpdateSerialize("(a+b)*(c+d)"), "(a+b)*(c+d)");
    EXPECT_EQ(ParseUpdateSerialize("b=string(false)"), "b=string(false)");
}

TEST(Parser, Single_Instruction_With_EndOfInstruction )
{
    EXPECT_TRUE(Parser_Test("a = 5;", double(5)));
    EXPECT_EQ(ParseUpdateSerialize("a = 5;"), "a = 5;");
}

TEST(Parser, Multiple_Instructions_Single_Line )
{
    EXPECT_TRUE(Parser_Test("a = 5;b = 2 * 5;", double(10)));
    EXPECT_EQ(ParseUpdateSerialize("a = 5;b = 2 * 5;"), "a = 5;b = 2 * 5;");
}

TEST(Parser, Multiple_Instructions_Multi_Line )
{
    EXPECT_TRUE(Parser_Test("a = 5;\nb = 2 * a;", double(10)));
    EXPECT_EQ(ParseUpdateSerialize("a = 5;\nb = 2 * a;"), "a = 5;\nb = 2 * a;");
    EXPECT_EQ(ParseUpdateSerialize("a = 5;b = 2 * a;\nc = 33 + 5;"), "a = 5;b = 2 * a;\nc = 33 + 5;");
}

TEST(Parser, DNAtoProtein )
{
    EXPECT_TRUE(Parser_Test("DNAtoProtein(\"TAA\")", "_"));
    EXPECT_TRUE(Parser_Test("DNAtoProtein(\"TAG\")", "_"));
    EXPECT_TRUE(Parser_Test("DNAtoProtein(\"TGA\")", "_"));
    EXPECT_TRUE(Parser_Test("DNAtoProtein(\"ATG\")", "M"));
}

TEST(Parser, Code_Formatting_Preserving )
{
    EXPECT_EQ(ParseUpdateSerialize("a =5;\nb=2*a;"), "a =5;\nb=2*a;");
    EXPECT_EQ(ParseUpdateSerialize("a =5;\nb=2  *  a;"), "a =5;\nb=2  *  a;");
    EXPECT_EQ(ParseUpdateSerialize(" 5 + 2;"), " 5 + 2;");
    EXPECT_EQ(ParseUpdateSerialize("5 + 2;  "), "5 + 2;  ");
}

TEST(Parser, Conditional_Structures_IF )
{
    EXPECT_EQ(ParseUpdateSerialize("if(sdfsd"), "");
    EXPECT_EQ(ParseUpdateSerialize("if(false){a=10;}"), "if(false){a=10;}");
    EXPECT_EQ(ParseUpdateSerialize("if (false){ a = 10; }"), "if (false){ a = 10; }");
    EXPECT_EQ(ParseUpdateSerialize("if (false){\n\ta = 10;\n}"), "if (false){\n\ta = 10;\n}");
    EXPECT_EQ(ParseUpdateSerialize("if(5 > 2){a=10;}"), "if(5 > 2){a=10;}");

    const char *program =
            "bob   = 10;"
            "alice = 10;"
            "if(bob > alice){"
            "   message = \"Bob is better than Alice.\";"
            "}";

    EXPECT_EQ(ParseUpdateSerialize(std::string(program)), std::string(program));

}

TEST(Parser, Conditional_Structures_IF_ELSE )
{
    EXPECT_EQ(ParseUpdateSerialize("if(false){a=10;}else"), "");
    EXPECT_EQ(ParseUpdateSerialize("if(false){a=10;}else{a=9;}"), "if(false){a=10;}else{a=9;}");

    const char *program =
            "bob   = 10;"
            "alice = 10;"
            "if(bob > alice){"
            "   message = \"Bob is the best.\";"
            "}else{"
            "   message = \"Bob is not the best.\";"
            "}";

    EXPECT_EQ(ParseUpdateSerialize(std::string(program)), std::string(program));
}

TEST(Parser, Conditional_Structures_IF_ELSE_IF )
{
    EXPECT_EQ(ParseUpdateSerialize("if(false){a=10;}else if"), "");
    EXPECT_EQ(ParseUpdateSerialize("if(false){a=10;}else if(false){a=9;}"), "if(false){a=10;}else if(false){a=9;}");

    const char *program =
            "bob   = 10;"
            "alice = 10;"
            "if (bob > alice){"
            "   message = \"Bob is greater than Alice.\";"
            "} else if (bob < alice ){"
            "   message = \"Bob is lower than Alice.\";"
            "} else {"
            "   message = \"Bob and Alice are equals.\";"
            "}";

    EXPECT_EQ(ParseUpdateSerialize(std::string(program)), std::string(program));

}