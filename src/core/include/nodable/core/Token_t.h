#pragma once
#include "nodable/core/types.h"

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

        ignore ,
        keyword_if,
        keyword_else,
        keyword_for,
        keyword_operator,
        keyword_string,
        keyword_double,
        keyword_int,
        keyword_bool,
        literal,
        operator_,
        identifier,
        open_bracket,
        close_bracket,
        separator,
        begin_scope,
        end_scope ,
        end_of_instruction,
        end_of_line,
        null, // to say 'absence of token', not token 'NULL'
        default_ = unknown,
        COUNT,
    };

	static constexpr bool is_keyword_type(Token_t _token_t)
    {
        switch (_token_t)
        {
            //------------keyword_<type>
            case Token_t::keyword_string:
            case Token_t::keyword_double:
            case Token_t::keyword_int:
            case Token_t::keyword_bool:
                return true;

            case Token_t::unknown: // -- I do not use default on purpose.
            case Token_t::ignore:  //    I want a warning when I add a new Token_t (especially keyword_<type>)
            case Token_t::keyword_if:
            case Token_t::keyword_else:
            case Token_t::keyword_for:
            case Token_t::keyword_operator:
            case Token_t::literal:
            case Token_t::operator_:
            case Token_t::identifier:
            case Token_t::open_bracket:
            case Token_t::close_bracket:
            case Token_t::separator:
            case Token_t::begin_scope:
            case Token_t::end_scope:
            case Token_t::end_of_instruction:
            case Token_t::end_of_line:
            case Token_t::null:
                return false;

        }
    }
}

