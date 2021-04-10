#pragma once

#include "Language/Common/TokenType.h"
#include <string>
#include <utility>
#include <vector>

namespace Nodable {

	struct Token
	{
		TokenType  type;
		std::string word;
		size_t charIndex; // the index of the first character of the token in the evaluated expression.
        std::string prefix; // additional text only useful for layout (spaces, tabs, new line, etc.)
        std::string suffix; // additional text only useful for layout (spaces, tabs, new line, etc.)

        Token(TokenType _type = TokenType_Default): Token(_type, "", 0) {}
		Token(TokenType _type, const std::string& _word, size_t _index):
		    type(_type),
		    word(_word),
            charIndex(_index)
        {}
        ~Token() = default;
		[[nodiscard]] static bool isOperand(TokenType type);
        static std::string toString(const Token* _token);
        static const Token Null;
    };
}