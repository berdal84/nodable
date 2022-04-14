#include <gtest/gtest.h>

#include <nodable/core/Member.h>
#include <nodable/core/VirtualMachine.h>
#include <nodable/core/GraphNode.h>
#include <nodable/core/VariableNode.h>
#include <nodable/core/NodeFactory.h>
#include <nodable/core/languages/Nodable.h>

using namespace Nodable;

class nodable_semantic_fixture: public ::testing::Test {
public:
    nodable_semantic_fixture( )
    : semantic(nodable_language.get_semantic())
    {
    }

    void SetUp( ) {
        // code here will execute just before the test ensues
    }

    void TearDown( ) {
        // code here will be called just after the test completes
        // ok to through exceptions from here if need be
    }

    ~nodable_semantic_fixture( )  {
        // cleanup any pending stuff, but no exceptions allowed
    }

    const LanguageNodable nodable_language;
    const Semantic&       semantic;
};

TEST_F(nodable_semantic_fixture, token_type_to_type)
{
    EXPECT_EQ(semantic.token_type_to_type(Token_t::keyword_bool)  , type::get<bool>());
    EXPECT_EQ(semantic.token_type_to_type(Token_t::keyword_double), type::get<double>() );
    EXPECT_EQ(semantic.token_type_to_type(Token_t::keyword_int)   , type::get<int>() );
    EXPECT_EQ(semantic.token_type_to_type(Token_t::keyword_int)   , type::get<i16_t>() );
    EXPECT_EQ(semantic.token_type_to_type(Token_t::keyword_string), type::get<std::string>() );
}

TEST_F(nodable_semantic_fixture, type_to_string)
{
    EXPECT_EQ(semantic.type_to_string(type::get<bool>())       , "bool" );
    EXPECT_EQ(semantic.type_to_string(type::get<double>())     , "double" );
    EXPECT_EQ(semantic.type_to_string(type::get<i16_t>())      , "int" );
    EXPECT_EQ(semantic.type_to_string(type::get<std::string>()), "string" );
}
