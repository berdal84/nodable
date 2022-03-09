#include <gtest/gtest.h>

#include <nodable/Member.h>
#include <nodable/VM.h>
#include <nodable/GraphNode.h>
#include <nodable/VariableNode.h>
#include <nodable/HeadlessNodeFactory.h>
#include <nodable/LanguageNodable.h>

using namespace Nodable;
using namespace Nodable::R;

TEST(Semantic, token_type_to_type)
{
    const LanguageNodable lang;
    auto semantic = lang.getSemantic();

    EXPECT_EQ(semantic->token_type_to_type(TokenType_KeywordBoolean), Type::Boolean );
    EXPECT_EQ(semantic->token_type_to_type(TokenType_KeywordDouble), Type::Double );
    EXPECT_EQ(semantic->token_type_to_type(TokenType_KeywordString), Type::String );
}

TEST(Semantic, type_to_string)
{
    const LanguageNodable lang;
    auto semantic = lang.getSemantic();

    EXPECT_EQ(semantic->type_to_string(R::get_meta_type<bool>() ), "bool" );
    EXPECT_EQ(semantic->type_to_string(R::get_meta_type<double>() ), "double" );
    EXPECT_EQ(semantic->type_to_string(R::get_meta_type<std::string>() ), "string" );
}
