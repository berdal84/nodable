#include "gtest/gtest.h"

#include "../../../src/core/include/nodable/Member.h"
#include "../../../src/core/include/nodable/Runner.h"

#include "../../../src/core/include/nodable/GraphNode.h"
#include "../../../src/core/include/nodable/Parser.h"
#include "../../../src/core/include/nodable/LanguageFactory.h"
#include "../../../src/core/include/nodable/VariableNode.h"
#include "../../../src/core/include/nodable/ScopedCodeBlockNode.h"
#include "../../../src/core/include/nodable/InstructionNode.h"
#include "../../../src/core/include/nodable/ProgramNode.h"
#include "../../../src/core/include/nodable/HeadlessNodeFactory.h"

using namespace Nodable::core;

template <typename T>
T ParseAndEvalExpression(const std::string& expression)
{
    // prepare
    bool success = false;
    const Language* lang = LanguageFactory::GetNodable();
    HeadlessNodeFactory factory(lang);
    GraphNode graph(lang, &factory );

    // act
    lang->getParser()->expressionToGraph(expression, &graph);

    T result{};
    if ( auto program = graph.getProgram())
    {
        // run
        Runner runner;
        runner.load(graph.getProgram());
        runner.run();

        // compare result
        auto lastInstruction = program->getLastInstruction();
        if ( lastInstruction )
        {
            result = (T)*lastInstruction->getValue();
        }

    }
    return result;
}


std::string& ParseUpdateSerialize( std::string& result, const std::string& expression )
{
    // prepare
    const Language* lang = LanguageFactory::GetNodable();
    HeadlessNodeFactory factory(lang);
    GraphNode graph(lang, &factory );

    // act
    lang->getParser()->expressionToGraph(expression, &graph);
    if ( ProgramNode* program = graph.getProgram())
    {
        Runner runner;
        runner.load(program);
        runner.run();

        if ( auto lastEvaluatedNode = runner.getLastEvaluatedInstruction() )
        {
            std::string result_str;
            lang->getSerializer()->serialize(result_str, lastEvaluatedNode->getValue()->getData() );
            LOG_MESSAGE("Test_Parser", "ParseUpdateSerialize result is: %s\n", result_str.c_str());
        }
    }

    Serializer* serializer = lang->getSerializer();
    serializer->serialize(result, graph.getProgram());

    std::cout << result << std::endl;

    return result;
}

void ParseEvalSerializeExpressions(const std::vector<std::string>& expressions)
{
    for ( const auto& original_expr : expressions )
    {
        std::string result_expr;
        ParseUpdateSerialize(result_expr, original_expr);
        EXPECT_EQ( result_expr, original_expr);
    }
}


TEST(Parser, Simple_expressions)
{
    EXPECT_EQ(ParseAndEvalExpression<int>("-5"), -5);
    EXPECT_EQ(ParseAndEvalExpression<int>("2+3"), 5);
    EXPECT_EQ(ParseAndEvalExpression<int>("-5+4"), -1);
    EXPECT_EQ(ParseAndEvalExpression<double>("-1+2*5-3/6"), 8.5);
}

TEST(Parser, Simple_parenthesis)
{
    EXPECT_EQ(ParseAndEvalExpression<int>("(1+4)"), 5);
    EXPECT_EQ(ParseAndEvalExpression<int>("(1)+(2)"), 3);
    EXPECT_EQ(ParseAndEvalExpression<int>("(1+2)*3"), 9);
    EXPECT_EQ(ParseAndEvalExpression<int>("2*(5+3)"), 16);
}

TEST(Parser, Unary_operators)
{
    EXPECT_EQ(ParseAndEvalExpression<int>("-1*20"), -20);
    EXPECT_EQ(ParseAndEvalExpression<int>("-(1+4)"), -5);
    EXPECT_EQ(ParseAndEvalExpression<int>("(-1)+(-2)"), -3);
    EXPECT_EQ(ParseAndEvalExpression<int>("-5*3"), -15);
    EXPECT_EQ(ParseAndEvalExpression<int>("2-(5+3)"), -6);
}

TEST(Parser, Complex_parenthesis)
{
    EXPECT_EQ(ParseAndEvalExpression<int>("2+(5*3)"), 2 + (5 * 3));
    EXPECT_EQ(ParseAndEvalExpression<int>("2*(5+3)+2"), 2 * (5 + 3) + 2);
    EXPECT_EQ(ParseAndEvalExpression<int>("(2-(5+3))-2+(1+1)"), (2 - (5 + 3)) - 2 + (1 + 1));
    EXPECT_EQ(ParseAndEvalExpression<double>("(2 -(5+3 )-2)+9/(1- 0.54)"), (2 - (5 + 3) - 2) + 9 / (1 - 0.54));
    EXPECT_EQ(ParseAndEvalExpression<double>("1/3"), 1.0 / 3.0);
}

TEST(Parser, Function_call)
{
    EXPECT_EQ(ParseAndEvalExpression<int>("returnNumber(5)"), 5);
    EXPECT_EQ(ParseAndEvalExpression<int>("returnNumber(1)"), 1);
    EXPECT_EQ(ParseAndEvalExpression<int>("sqrt(81)"), 9);
    EXPECT_EQ(ParseAndEvalExpression<int>("pow(2,2)"), 4);
}

TEST(Parser, FunctionLike_operators_call)
{
    EXPECT_EQ(ParseAndEvalExpression<double>("operator*(2,2)"), 4.0);
    EXPECT_EQ(ParseAndEvalExpression<bool>("operator>(2,2)"), false);
    EXPECT_EQ(ParseAndEvalExpression<double>("operator-(3,2)"), 1.0);
    EXPECT_EQ(ParseAndEvalExpression<double>("operator+(2,2)"), 4.0);
    EXPECT_EQ(ParseAndEvalExpression<double>("operator/(4,2)"), 2.0);
}

TEST(Parser, Imbricated_functions)
{
    EXPECT_EQ(ParseAndEvalExpression<int>("returnNumber(5+3)"), 8);
    EXPECT_EQ(ParseAndEvalExpression<int>("returnNumber(returnNumber(1))"), 1);
    EXPECT_EQ(ParseAndEvalExpression<int>("returnNumber(returnNumber(1) + returnNumber(1))"), 2);
}

TEST(Parser, Successive_assigns)
{
    EXPECT_EQ(ParseAndEvalExpression<double>("double a; double b; a = b = 5;"), 5.0);
}

TEST(Parser, Strings)
{
    EXPECT_EQ(ParseAndEvalExpression<std::string>("string a = \"coucou\""), "coucou");
    EXPECT_EQ(ParseAndEvalExpression<std::string>("string a = to_string(15)"), "15");
    EXPECT_EQ(ParseAndEvalExpression<std::string>("string a = to_string(-15)"), "-15");
    EXPECT_EQ(ParseAndEvalExpression<std::string>("string a = to_string(-15.5)"), "-15.5");
    EXPECT_EQ(ParseAndEvalExpression<std::string>("string b = to_string(true)"), "true");
    EXPECT_EQ(ParseAndEvalExpression<std::string>("string b = to_string(false)"), "false");
}

TEST(Parser, Serialize_Precedence)
{
    std::vector<std::string> expressions
    {
        "(1+1)*2",
        "1*1+2",
        "-(-1)",
        "-(2*5)",
        "(-2)*5",
        "-(2+5)",
        "5+(-1)*3"
    };

    ParseEvalSerializeExpressions(expressions);
}

TEST(Parser, Eval_Serialize_Compare)
{
    std::vector<std::string> expressions
    {
            "1",
            "1+1",
            "1-1",
            "-1",
            "double a = 5",
            "(a+b)*(c+d)",
            "string b = to_string(false)"
    };

    ParseEvalSerializeExpressions(expressions);
}

TEST(Parser, Declare_and_define_vars)
{
    std::vector<std::string> expressions
    {
        "double a = 10.5;",
        R"(string s = "coucou";)",
        "bool b = false;"
    };

    ParseEvalSerializeExpressions(expressions);
}

TEST(Parser, Single_Instruction_With_EndOfInstruction )
{
    EXPECT_EQ(ParseAndEvalExpression<double>("double a = 5;"), 5.0);

    std::vector<std::string> expressions { "double a = 5;" };
    ParseEvalSerializeExpressions(expressions);
}

TEST(Parser, Multiple_Instructions_Single_Line )
{
    EXPECT_EQ(ParseAndEvalExpression<double>("double a = 5;double b = 2 * 5;"), 10.0 );

    std::vector<std::string> expressions { "double a = 5;double b = 2 * 5;" };
    ParseEvalSerializeExpressions(expressions);
}

TEST(Parser, Multiple_Instructions_Multi_Line )
{
    EXPECT_EQ(ParseAndEvalExpression<double>("double a = 5;\ndouble b = 2 * a;"), 10.0 );

    std::vector<std::string> expressions {
        "double a = 5;\ndouble b = 2 * a;",
        "double a = 5;double b = 2 * a;\ndouble c = 33 + 5;"
    };
    ParseEvalSerializeExpressions(expressions);
}

TEST(Parser, DNAtoProtein )
{
    EXPECT_EQ(ParseAndEvalExpression<std::string>("DNAtoProtein(\"TAA\")"), "_");
    EXPECT_EQ(ParseAndEvalExpression<std::string>("DNAtoProtein(\"TAG\")"), "_");
    EXPECT_EQ(ParseAndEvalExpression<std::string>("DNAtoProtein(\"TGA\")"), "_");
    EXPECT_EQ(ParseAndEvalExpression<std::string>("DNAtoProtein(\"ATG\")"), "M");
}

TEST(Parser, Code_Formatting_Preserving )
{
    std::vector<std::string> expressions {
            "a =5;\nb=2*a;",
            "a =5;\nb=2  *  a;",
            " 5 + 2;",
            "5 + 2;  "
    };
    ParseEvalSerializeExpressions(expressions);
}

TEST(Parser, Conditional_Structures_IF )
{
    std::string program =
            "double bob   = 10;"
            "double alice = 10;"
            "if(bob>alice){"
            "   string message = \"Bob is better than Alice.\";"
            "}";

    ParseEvalSerializeExpressions({program});
}

TEST(Parser, Conditional_Structures_IF_ELSE )
{
    std::string program =
            "double bob   = 10;"
            "double alice = 10;"
            "string message;"
            "if(bob>alice){"
            "   message= \"Bob is the best.\";"
            "}else{"
            "   message= \"Bob is not the best.\";"
            "}";

    ParseEvalSerializeExpressions({program});
}

TEST(Parser, Conditional_Structures_IF_ELSE_IF )
{
    std::string program =
            "double bob   = 10;"
            "double alice = 10;"
            "string message;"
            "if(bob>alice){"
            "   message= \"Bob is greater than Alice.\";"
            "} else if(bob<alice){"
            "   message= \"Bob is lower than Alice.\";"
            "} else {"
            "   message= \"Bob and Alice are equals.\";"
            "}";

    ParseEvalSerializeExpressions({program});
}