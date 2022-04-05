#include <gtest/gtest.h>

#include <nodable/core/Member.h>
#include <nodable/core/VM.h>
#include <nodable/core/GraphNode.h>
#include <nodable/core/VariableNode.h>
#include <nodable/core/NodeFactory.h>
#include <nodable/core/languages/Nodable.h>

using namespace Nodable;
using namespace Nodable::R;

TEST(Semantic, token_type_to_type)
{
    const LanguageNodable lang;
    auto semantic = lang.get_semantic();

    EXPECT_EQ(semantic->token_type_to_type(Token_t::keyword_bool)  , Type::bool_t );
    EXPECT_EQ(semantic->token_type_to_type(Token_t::keyword_double), Type::double_t );
    EXPECT_EQ(semantic->token_type_to_type(Token_t::keyword_int)   , Type::i16_t );
    EXPECT_EQ(semantic->token_type_to_type(Token_t::keyword_string), Type::string_t );
}

TEST(Semantic, type_to_string)
{
    const LanguageNodable lang;
    auto semantic = lang.get_semantic();

    EXPECT_EQ(semantic->type_to_string(R::get_meta_type<bool>() )       , "bool" );
    EXPECT_EQ(semantic->type_to_string(R::get_meta_type<double>() )     , "double" );
    EXPECT_EQ(semantic->type_to_string(R::get_meta_type<i16_t>() )      , "int" );
    EXPECT_EQ(semantic->type_to_string(R::get_meta_type<std::string>() ), "string" );
}
