#pragma once

/*
	This enum identifies each Type of Token that a Language should handle in its Semantic.

	ex: 
		C++ Language will define "bool" as BooleanType
		TypeScript Language will define "boolean" as BooleanType

	(cf. Semantic class)
*/

namespace Nodable {

    /**
     * Note: the order has an impact on parsing.
     *       I should fix that. Language derived must define the parsing priorities.
     */
	enum class TokenType
	{
	    Any,
		Unknown,
        KeywordIf,
        KeywordElse,
        Ignore,
        EndOfInstruction,
        EndOfLine,
		AnyType,
		String,
		Boolean,
		Double,
		Operator,

		OpenBracket,
		CloseBracket,
        BeginScope,
        EndScope,

		Separator,
		Space,
        Symbol,
        StringType,
        DoubleType,
        BooleanType,
        KeywordOperator,
		Default = Unknown,
    };
}
