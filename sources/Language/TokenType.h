#pragma once

/*
	This enum identifies each Type of Token that a Language should handle in its Dictionnary.

	ex: 
		C++ Language will define "bool" as BooleanType
		TypeScript Language will define "boolean" as BooleanType

	(cf. Dictionnary class)
*/

namespace Nodable {

	enum class TokenType
	{
		Unknown,		

		StringType,
		DoubleType,
		BooleanType,
		AnyType,

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
