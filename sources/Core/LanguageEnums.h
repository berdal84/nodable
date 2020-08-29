#pragma once

/* This enum identifies each kind of word for a language */

namespace Nodable {

	enum class TokenType
	{
		Str,
		Double,
		Symbol,
		Operator,
		Bool,
		Bracket,
		Comma,
		COUNT,
		Unknown
	};
}
