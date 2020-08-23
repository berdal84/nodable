#pragma once

/* This enum identifies each kind of word for a language */

namespace Nodable {

	enum TokenType_
	{
		TokenType_String,
		TokenType_Number,
		TokenType_Symbol,
		TokenType_Operator,
		TokenType_Boolean,
		TokenType_Parenthesis,
		TokenType_Comma,
		TokenType_COUNT,
		TokenType_Unknown
	};
}
