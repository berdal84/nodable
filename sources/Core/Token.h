#pragma once

#include "Language/Common/TokenType.h"
#include <string>
#include <vector>

namespace Nodable {

	class Token
	{
	public:
		TokenType  type;
		std::string word;
		size_t charIndex; // the index of the first character of the token in the evaluated expression.
        std::string prefix; // additional text only useful for layout (spaces, tabs, new line, etc.)
        std::string suffix; // additional text only useful for layout (spaces, tabs, new line, etc.)

        Token(TokenType _type): Token(_type, "", 0) {}

		Token(TokenType _type, const std::string& _word, size_t _index):
		    type(_type),
		    word(_word),
            charIndex(_index)
        {}

		bool isOperand()const { // TODO: move this into "Parser" or "Language"
			return type == TokenType::Double || type == TokenType::Boolean ||
				   type == TokenType::String || type == TokenType::Symbol;
		}

		/**
		 * Convert a token to a string using the pattern: "{ word: "<word>", charIndex: <index> }"
		 * @return
		 */
        std::string toString()const
        {
            std::string result;
            result.append("{ ");
            result.append( "word: " + this->word );
            result.append( ", charIndex: " + std::to_string(this->charIndex) );
            result.append(" }");
            return result;
        }
    };
}