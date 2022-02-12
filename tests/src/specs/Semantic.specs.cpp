#include <gtest/gtest.h>

#include <nodable/Member.h>
#include <nodable/VM.h>
#include <nodable/GraphNode.h>
#include <nodable/VariableNode.h>
#include <nodable/HeadlessNodeFactory.h>
#include <nodable/LanguageNodable.h>

using namespace Nodable;
using namespace Nodable::Reflect;

TEST(Semantic, token_type_to_type)
{
    const LanguageNodable lang;
    auto semantic = lang.getSemantic();

    EXPECT_EQ( semantic->token_type_to_type(TokenType_KeywordBoolean), Type_Boolean );
    EXPECT_EQ( semantic->token_type_to_type(TokenType_KeywordDouble), Type_Double );
    EXPECT_EQ( semantic->token_type_to_type(TokenType_KeywordString), Type_String );
}

TEST(Semantic, type_to_string)
{
    const LanguageNodable lang;
    auto semantic = lang.getSemantic();

    EXPECT_EQ( semantic->type_to_string(Type_Boolean), "bool" );
    EXPECT_EQ( semantic->type_to_string(Type_Double), "double" );
    EXPECT_EQ( semantic->type_to_string(Type_String), "string" );
}
