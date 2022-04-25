#pragma once
#include "nodable/core/types.h"

/*
	This enum identifies each Type of Token that a Language should handle in its Semantic.

	ex: 
		C++ Language will define        bool    as KeywordBoolean
		TypeScript Language will define boolean as KeywordBoolean

	(cf. Semantic class)
*/

namespace ndbl {


	enum class Token_t: int
	{
        unknown,

        ignore ,
        keyword_if,
        keyword_else,
        keyword_for,
        keyword_operator,
        keyword_string,
        keyword_double,
        keyword_int,
        keyword_bool,
        literal_string,
        literal_double,
        literal_int,
        literal_bool,
        operator_,
        identifier,
        fct_params_begin,
        fct_params_end,
        fct_params_separator,
        scope_begin,
        scope_end ,
        end_of_instruction,
        end_of_line,

        COUNT,

        null, // to say 'absence of token', not token 'NULL'
        default_ = unknown,
    };

	static constexpr bool is_keyword_type(Token_t _token_t)
    {
        return _token_t >= Token_t::keyword_string && _token_t <= Token_t::keyword_bool;
    }

}

