#include <gtest/gtest.h>

#include <nodable/core/Member.h>
#include <nodable/core/VirtualMachine.h>
#include <nodable/core/GraphNode.h>
#include <nodable/core/VariableNode.h>
#include <nodable/core/NodeFactory.h>
#include <nodable/core/languages/Nodable.h>

using namespace Nodable;

TEST(Semantic, token_type_to_type)
{
    const LanguageNodable lang;
    const Semantic& semantic = lang.get_semantic();

    EXPECT_EQ(semantic.token_type_to_type(Token_t::keyword_bool)  , type::get<bool>());
    EXPECT_EQ(semantic.token_type_to_type(Token_t::keyword_double), type::get<double>() );
    EXPECT_EQ(semantic.token_type_to_type(Token_t::keyword_int)   , type::get<i16_t>() );
    EXPECT_EQ(semantic.token_type_to_type(Token_t::keyword_string), type::get<std::string>() );
}

TEST(Semantic, type_to_string)
{
    const LanguageNodable lang;
    const Semantic& semantic = lang.get_semantic();

    EXPECT_EQ(semantic.type_to_string(type::get<bool>())       , "bool" );
    EXPECT_EQ(semantic.type_to_string(type::get<double>())     , "double" );
    EXPECT_EQ(semantic.type_to_string(type::get<i16_t>())      , "int" );
    EXPECT_EQ(semantic.type_to_string(type::get<std::string>()), "string" );
}
