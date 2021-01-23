#pragma once

#include "Language/Common/TokenType.h"
#include <string>

namespace Nodable {

	class Token
	{
	public:
		TokenType  type;
		std::string word;
		size_t charIndex; // the index of the first character of the token in the evaluated expression.

		Token(TokenType _type, const std::string& _word, size_t _index):
		    type(_type),
		    word(_word),
            charIndex(_index)
        {}

		bool isOperand()const { // TODO: move this into "Parser" or "Language"
			return type == TokenType::Double || type == TokenType::Boolean ||
				   type == TokenType::String || type == TokenType::Symbol;
		}

	};
}