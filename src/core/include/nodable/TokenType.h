#pragma once

/*
	This enum identifies each Type of Token that a Language should handle in its Semantic.

	ex: 
		C++ Language will define        bool    as TokenType_KeywordBoolean
		TypeScript Language will define boolean as TokenType_KeywordBoolean

	(cf. Semantic class)
*/

namespace Nodable {


	enum TokenType
	{
        TokenType_Unknown,

        TokenType_Ignore,

        TokenType_KeywordIf,          // if
        TokenType_KeywordElse,
        TokenType_KeywordString,
        TokenType_KeywordDouble,
        TokenType_KeywordBoolean,
        TokenType_KeywordOperator,
        TokenType_KeywordAny,

        TokenType_Literal,            // 5.0, "coucou" or true
        TokenType_Operator,           // ... operator ..., ex 5 + 7
        TokenType_Identifier,         // function() or variable

        TokenType_OpenBracket,        // ( )
        TokenType_CloseBracket,
        TokenType_Separator,          // , ex: call(a , b, c, d, ... )

        TokenType_BeginScope,         // { }
        TokenType_EndScope,
        TokenType_EndOfInstruction,   // ;
        TokenType_EndOfLine,          // \n

        TokenType_COUNT,

		TokenType_NULL, // to say 'absence of token', not token 'NULL'
        TokenType_Default = TokenType_Unknown,
    };
}

