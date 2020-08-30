#pragma once

/* This enum identifies each kind of word for a language */

namespace Nodable {

	enum class TokenType
	{
		Unknown   = 1 << 0,
		Str       = 1 << 1,
		Double    = 1 << 2,
		Symbol    = 1 << 3,
		Operator  = 1 << 4,
		Bool      = 1 << 5,
		LBracket  = 1 << 6,
		RBracket  = 1 << 7, 
		Separator = 1 << 8,
		Space     = 1 << 9,
		Tab       = 1 << 10
	};
}
