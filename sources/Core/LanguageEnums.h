#pragma once

/* This enum identifies each kind of word for a language */

namespace Nodable {

	enum class TokenType: uint16_t
	{
		Unknown,
		Str,
		Double,
		Symbol,
		Operator,
		Bool,
		LBracket,
		RBracket, 
		Separator,
		Space,
		Comment,
		Tab
	};
}
