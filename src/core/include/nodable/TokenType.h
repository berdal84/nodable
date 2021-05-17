#pragma once

/*
	This enum identifies each Type of Token that a Language should handle in its Semantic.

	ex: 
		C++ Language will define "bool" as BooleanType
		TypeScript Language will define "boolean" as BooleanType

	(cf. Semantic class)
*/

namespace Nodable::core {

	enum TokenType
	{
        TokenType_Any = 0,
        TokenType_Unknown,
        TokenType_KeywordIf,
        TokenType_KeywordElse,
        TokenType_Ignore,
        TokenType_EndOfInstruction,
        TokenType_EndOfLine,
        TokenType_AnyType,
        TokenType_String,
        TokenType_Boolean,
        TokenType_Double,
        TokenType_Operator,
        TokenType_OpenBracket,
        TokenType_CloseBracket,
        TokenType_BeginScope,
        TokenType_EndScope,
        TokenType_Separator,
        TokenType_Space,
        TokenType_Identifier,
        TokenType_StringType,
        TokenType_DoubleType,
        TokenType_BooleanType,
        TokenType_KeywordOperator,

        TokenType_COUNT,

		TokenType_NULL, // to say 'absence of token', not token 'NULL'
        TokenType_Default = TokenType_Unknown,
    };
}
