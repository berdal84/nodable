#pragma once

#include <nodable/core/Token_t.h>
#include <string>
#include <utility>
#include <vector>
#include <memory>

namespace Nodable
{
	struct Token
	{
	    size_t      m_index;
		Token_t     m_type;
		std::string m_word;
		size_t      m_charIndex; // the index of the first character of the token in the evaluated expression.
        std::string m_prefix; // additional text only useful for layout (spaces, tabs, new line, etc.)
        std::string m_suffix; // additional text only useful for layout (spaces, tabs, new line, etc.)

        Token(Token_t _type = Token_t::default_): Token(_type, "", 0) {}
		Token(Token_t _type, const std::string& _word, size_t _char_index):
                m_type(_type),
                m_word(_word),
                m_index(0),
                m_charIndex(_char_index)
        {}
        ~Token() = default;
        void clear()
        {
            m_index = 0;
            m_type = Token_t::default_;
            m_word.clear();
            m_charIndex = 0;
            m_prefix.clear();
            m_suffix.clear();
        }
        static std::string to_string(std::shared_ptr<Token> _token);
        static const std::shared_ptr<Token> s_null;

        bool isOperand()
        {
            return m_type == Token_t::literal || m_type == Token_t::identifier;
        }

        bool isTypeKeyword()
        {
            return m_type == Token_t::keyword_double || m_type == Token_t::keyword_bool || m_type == Token_t::keyword_string;
        }

    };
}