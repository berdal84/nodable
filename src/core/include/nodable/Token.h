#pragma once

#include <nodable/TokenType.h>
#include <string>
#include <utility>
#include <vector>
#include <memory>

namespace Nodable
{
	struct Token
	{
	    size_t      m_index;
		TokenType   m_type;
		std::string m_word;
		size_t      m_charIndex; // the index of the first character of the token in the evaluated expression.
        std::string m_prefix; // additional text only useful for layout (spaces, tabs, new line, etc.)
        std::string m_suffix; // additional text only useful for layout (spaces, tabs, new line, etc.)

        Token(TokenType _type = TokenType_Default): Token(_type, "", 0) {}
		Token(TokenType _type, const std::string& _word, size_t _char_index):
                m_type(_type),
                m_word(_word),
                m_index(0),
                m_charIndex(_char_index)
        {}
        ~Token() = default;

        static std::string to_string(std::shared_ptr<Token> _token);
        static const std::shared_ptr<Token> s_null;

        static inline bool isOperand(TokenType type)
        {
            return type == TokenType_Literal || type == TokenType_Identifier;
        }

        static inline bool isType(TokenType type)
        {
            return type == TokenType_KeywordDouble || type == TokenType_KeywordBoolean || type == TokenType_KeywordString;
        }

    };
}