#pragma once

/* This enum identifies each kind of word for a language */

namespace Nodable {

	enum class TokenType
	{
		Unknown,		

		StringType,
		DoubleType,
		BooleanType,

		String,
		Boolean,
		Double,

		Operator,
		LBracket,
		RBracket, 
		Separator,
		Space,
		Ignore,
		EndOfInstruction,
		Symbol

	};
}
