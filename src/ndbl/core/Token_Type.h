#pragma once
#include "bdc/Types.hpp"

namespace ndbl {

    /**
     * @enum Identifies each Type of Token that a Language should state.
     * @note When the parser find a match, it assign a given Token_t to the parsed token.
     * @example
     *     "bool" => Token_t::keyword_bool
     *     "100"  => Token_t::literal_int
     */
    typedef int Token_Type;
	enum Token_Type_: Token_Type
	{
        Token_Type_NULL = 0, // to say 'absence of token'

        Token_Type_ignore,
        Token_Type_keyword_if,
        Token_Type_keyword_else,
        Token_Type_keyword_for,
        Token_Type_keyword_while,
        Token_Type_keyword_operator,
        Token_Type_keyword_return,
        //----- types -------
        Token_Type_keyword_string,
        Token_Type_keyword_double,
        Token_Type_keyword_int,
        Token_Type_keyword_i16,
        Token_Type_keyword_bool,
        Token_Type_keyword_any,     // like TypeScript's
        Token_Type_keyword_unknown, // like TypeScript's
        Token_Type_keyword_FIRST  = Token_Type_keyword_string,
        Token_Type_keyword_LAST   = Token_Type_keyword_unknown,
        //----- literals -----
        Token_Type_literal_string,
        Token_Type_literal_double,
        Token_Type_literal_int,
        Token_Type_literal_bool,
        Token_Type_literal_any,
        Token_Type_literal_unknown,
        Token_Type_operator,
        Token_Type_identifier,
        Token_Type_parenthesis_open,
        Token_Type_parenthesis_close,
        Token_Type_list_separator,
        Token_Type_scope_begin,
        Token_Type_scope_end ,
        Token_Type_end_of_instruction,
        Token_Type_end_of_line,
    };

    /** Check if a given keyword is a type (ex: bool, int, double,...)*/
	static constexpr bool is_a_type_keyword(Token_Type _token_t)
    {
        return Token_Type_keyword_FIRST <= _token_t && _token_t <= Token_Type_keyword_LAST;
    }

}

