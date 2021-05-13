#pragma once

#include "Language/Common/TokenType.h"
#include <string>
#include <utility>
#include <vector>

namespace Nodable
{
	struct Token
	{
		TokenType   m_type;
		std::string m_word;
		size_t      m_charIndex; // the index of the first character of the token in the evaluated expression.
        std::string m_prefix; // additional text only useful for layout (spaces, tabs, new line, etc.)
        std::string m_suffix; // additional text only useful for layout (spaces, tabs, new line, etc.)

        Token(TokenType _type = TokenType_Default): Token(_type, "", 0) {}
		Token(TokenType _type, const std::string& _word, size_t _index):
                m_type(_type),
                m_word(_word),
                m_charIndex(_index)
        {}
        ~Token() = default;

        static std::string toString(const Token* _token);
        static const Token s_null;

        static inline bool isOperand(TokenType type)
        {
            return type == TokenType_Double || type == TokenType_Boolean || type == TokenType_String || type == TokenType_Identifier;
        }

        static inline bool isType(TokenType type)
        {
            return type == TokenType_DoubleType || type == TokenType_BooleanType || type == TokenType_StringType;
        }

    };
}