#pragma once
#include "tools/core/types.h"

namespace ndbl {

    /**
     * @enum Identifies each Type of Token that a Language should state.
     * @note When the parser find a match, it assign a given Token_t to the parsed token.
     * @example
     *     "bool" => Token_t::keyword_bool
     *     "100"  => Token_t::literal_int
     */
	enum class Token_t: i8_t
	{
        null = 0, // to say 'absence of token', not token 'NULL'

        ignore ,
        keyword_if,
        keyword_else,
        keyword_for,
        keyword_while,
        keyword_operator,
        keyword_string,
        keyword_double,
        keyword_int,
        keyword_i16,
        keyword_bool,
        literal_string,
        literal_double,
        literal_int,
        literal_bool,
        operator_,
        identifier,
        parenthesis_open,
        parenthesis_close,
        list_separator,
        scope_begin,
        scope_end ,
        end_of_instruction,
        end_of_line,

        COUNT,
    };

    /** Check if a given keyword is a type (ex: bool, int, double,...)*/
	static constexpr bool is_a_type_keyword(Token_t _token_t)
    {
        return _token_t >= Token_t::keyword_string && _token_t <= Token_t::keyword_bool;
    }

}

