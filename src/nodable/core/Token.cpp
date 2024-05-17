#include "Token.h"
#include <cstring>

using namespace ndbl;

const Token Token::s_null(Token_t::null);
const Token Token::s_end_of_line{Token_t::ignore, "\n"};
const Token Token::s_end_of_instruction{Token_t::ignore, ";\n"};

std::string Token::json() const
{
    std::string result;
    result.append("{ ");
    result.append("word: \"" + word_to_string() + "\"" );
    result.append( ", charIndex: " + std::to_string(m_buffer_start_pos) );
    result.append(", prefix: \"" + prefix_to_string() + "\""  );
    result.append(", suffix: \"" + suffix_to_string() + "\""  );
    result.append(", word: \"" + word_to_string() + "\""  );
    result.append( ", token_type: \"" + std::to_string((int)m_type) + "\"");
    result.append(" }");
    return result;
}

void Token::move_prefixsuffix(Token* source)
{
    if( m_is_source_buffer_owned ) delete[] m_source_buffer;

    std::string word_copy{m_source_buffer != nullptr ? word_to_string() : ""};

    // transfer prefix and suffix to this token, but keep the same word.
    // this operation requires the buffer to be owned
    m_buffer_size =
            source->m_buffer_size - source->m_word_size //   source's prefix and suffix size
            + m_word_size;                              // + current word

    m_source_buffer = new char[m_buffer_size];
    m_is_source_buffer_owned = true;
    m_buffer_start_pos = 0;
    m_word_start_pos = 0;
    m_word_size = word_copy.length();

    // copy prefix from source
    if( size_t prefix_size = source->prefix_size())
    {
        memcpy(m_source_buffer, source->buffer(), prefix_size);
        m_word_start_pos = prefix_size;
    }

    // reassign word
    if( m_word_size )
    {
        memcpy(word(), word_copy.data(), m_word_size);
    }

    // copy suffix from source
    if( size_t suffix_size = source->suffix_size())
    {
        memcpy(suffix(), source->suffix(), suffix_size);
    }

    // Remove prefix and suffix on the source
    source->m_buffer_start_pos = source->m_word_start_pos;
    source->m_buffer_size = source->m_word_size;
}

void Token::clear()
{
    m_index = 0;
    m_type  = Token_t::null;
    m_buffer_start_pos = 0;
    m_buffer_size      = 0;
    m_word_start_pos   = 0;
    m_word_size        = 0;

    if( m_is_source_buffer_owned )
    {
        delete[] m_source_buffer;
        m_source_buffer = nullptr;
    }
}

void Token::set_source_buffer(char *_buffer, size_t pos, size_t size)
{
    if( m_is_source_buffer_owned ) delete[] m_source_buffer;

    m_source_buffer = _buffer;
    m_buffer_start_pos = m_word_start_pos = pos;
    m_buffer_size = m_word_size = size;
}

std::string Token::prefix_to_string()const
{
    if( has_buffer() )
    {
        return std::string{ prefix(), prefix_size()};
    }
    return {};
}

std::string Token::word_to_string()const
{
    if( has_buffer() )
    {
        return std::string{ word(), m_word_size};
    }
    return {};
}

std::string Token::suffix_to_string()const
{
    if( has_buffer() )
    {
        return std::string{ suffix(), suffix_size()};
    }
    return {};
}

std::string Token::buffer_to_string() const
{
    if (has_buffer())
    {
        return { buffer(), m_buffer_size };
    }
    return {};
}


Token::Token(Token&& other)
{
    *this = std::move( other );
};

Token& Token::operator=(Token&& other) noexcept
{
    if( this == &other )
    {
        return *this;
    }

    m_source_buffer          = other.m_source_buffer;
    m_word_size              = other.m_word_size;
    m_buffer_size            = other.m_buffer_size;
    m_is_source_buffer_owned = other.m_is_source_buffer_owned;
    m_word_start_pos         = other.m_word_start_pos;
    m_buffer_start_pos       = other.m_buffer_start_pos;
    m_type                   = other.m_type;
    m_index                  = other.m_index;

    other.m_source_buffer          = nullptr;
    other.m_buffer_size            = 0;
    other.m_word_size              = 0;
    other.m_is_source_buffer_owned = false;
    other.m_buffer_start_pos       = 0;
    other.m_word_start_pos         = 0;
    other.m_buffer_start_pos       = 0;
    other.m_type                   = Token_t::null;
    other.m_index                  = 0;

    return *this;
};

Token& Token::operator=(const Token& other)
{
    if( this == &other) return *this;

    if( m_is_source_buffer_owned )
    {
        delete[] this->m_source_buffer;
    }

    m_index                  = other.m_index;
    m_buffer_start_pos       = other.m_buffer_start_pos;
    m_buffer_size            = other.m_buffer_size;
    m_word_start_pos         = other.m_word_start_pos;
    m_word_size              = other.m_word_size;
    m_type                   = other.m_type;
    m_is_source_buffer_owned = other.m_is_source_buffer_owned;

    if( other.m_is_source_buffer_owned )
    {
        m_source_buffer = new char[m_buffer_size];
        memcpy(m_source_buffer, other.m_source_buffer, m_buffer_size);
    }
    else
    {
        m_source_buffer = other.m_source_buffer;
    }

    return *this;
};