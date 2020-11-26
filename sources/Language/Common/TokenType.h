#pragma once

/*
	This enum identifies each Type of Token that a Language should handle in its Dictionnary.

	ex: 
		C++ Language will define "bool" as BooleanType
		TypeScript Language will define "boolean" as BooleanType

	(cf. Dictionnary class)
*/

namespace Nodable {

    /**
     * Note: the order has an impact on parsing.
     *       I should fix that. Language derived must define the parsing priorities.
     */
	enum class TokenType
	{
		Unknown,
        Ignore,
		AnyType,
		String,
		Boolean,
		Double,
		Operator,
		LBracket,
		RBracket, 
		Separator,
		Space,
        Symbol,
		EndOfInstruction,
        StringType,
        DoubleType,
        BooleanType,

		Default = Unknown

	};
}
