#pragma once

/*
	This enum identifies each Type of Token that a Language should handle in its Semantic.

	ex: 
		C++ Language will define        bool    as KeywordBoolean
		TypeScript Language will define boolean as KeywordBoolean

	(cf. Semantic class)
*/

namespace Nodable {


	enum class Token_t
	{
        unknown,

        ignore,

        keyword_if,
        keyword_else,
        keyword_for,
        keyword_string,
        keyword_double,
        keyword_int,
        keyword_bool,
        keyword_operator,

        literal,
        operator_,
        identifier,

        open_bracket,
        close_bracket,
        separator,

        begin_scope,
        end_scope,
        end_of_instruction,
        end_of_line,

        COUNT,

		null, // to say 'absence of token', not token 'NULL'
        default_ = unknown,
    };
}

