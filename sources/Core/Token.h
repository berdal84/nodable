#pragma once

#include "TokenType.h"
#include <string>

namespace Nodable {

	class Token
	{
	public:
		TokenType  type = TokenType::Default; // the type of the token
		std::string word = "";                // the word as a string
		size_t      charIndex = 0;                 // the index of the first character of the token in the evaluated expression.

		bool isOperand()const { // TODO: move this into "Parser" or "Language"
			return type == TokenType::DoubleType ||
				type == TokenType::BooleanType ||
				type == TokenType::StringType ||
				type == TokenType::Symbol;
		}

	};
}