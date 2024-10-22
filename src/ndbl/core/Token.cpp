#include "Token.h"
#include <cstring>

using namespace ndbl;

const Token Token::s_null               = {Token_t::null};
const Token Token::s_end_of_line        = {Token_t::ignore, "\n"};
const Token Token::s_end_of_instruction = {Token_t::ignore, ";\n"};

std::string Token::json() const
{
    std::string result;
    result.reserve(m_string_length);

    result.append("{ ");

    result.append("word: \"" );
    result.append(word_ptr(), word_size());
    result.append("\"" );

    result.append(", charIndex: " + std::to_string(m_string_start_pos) );

    result.append(", prefix: \""  );
    result.append(prefix_ptr(), prefix_size());
    result.append("\""  );

    result.append(", suffix: \""  );
    result.append(suffix_ptr(), suffix_size());
    result.append("\""  );

    result.append(", token_type: \"" + std::to_string((int)m_type) + "\"");

    result.append(" }");

    return result;
}

void Token::take_prefix_suffix_from(Token* source)
{
    if( m_is_buffer_owned ) delete[] m_buffer;

    std::string word_copy{m_buffer != nullptr ? word_to_string() : ""};

    // transfer prefix and suffix to this token, but keep the same word.
    // this operation requires the buffer to be owned
    m_string_length =
            source->m_string_length - source->m_word_length //   source's prefix and suffix size
            + m_word_length;                              // + current word

    m_buffer           = new char[m_string_length];
    m_is_buffer_owned  = true;
    m_string_start_pos = 0;
    m_word_start_pos   = 0;
    m_word_length      = word_copy.length();

    // copy prefix from source
    if( size_t prefix_size = source->prefix_size())
    {
        memcpy(m_buffer, source->string_ptr(), prefix_size);
        m_word_start_pos = prefix_size;
    }

    // reassign word
    if( m_word_length )
    {
        memcpy(word_ptr(), word_copy.data(), m_word_length);
    }

    // copy suffix from source
    if( size_t suffix_size = source->suffix_size())
    {
        memcpy(suffix_ptr(), source->suffix_ptr(), suffix_size);
    }

    // Remove prefix and suffix on the source
    source->m_string_start_pos = source->m_word_start_pos;
    source->m_string_length = source->m_word_length;
}

void Token::clear()
{
    m_index = 0;
    m_type  = Token_t::null;
    m_string_start_pos = 0;
    m_string_length      = 0;
    m_word_start_pos   = 0;
    m_word_length        = 0;

    if( m_is_buffer_owned )
    {
        delete[] m_buffer;
        m_buffer = nullptr;
    }
}

void Token::set_source_buffer(char *_buffer, size_t pos, size_t size)
{
    if( m_is_buffer_owned )
        delete[] m_buffer;

    m_buffer = _buffer;
    m_string_start_pos = m_word_start_pos = pos;
    m_string_length    = m_word_length = size;
}

std::string Token::prefix_to_string()const
{
    if( has_buffer() )
    {
        return std::string{prefix_ptr(), prefix_size()};
    }
    return {};
}

std::string Token::word_to_string()const
{
    if( has_buffer() )
    {
        ASSERT(m_word_length < 50); // are you sure?
        return std::string{word_ptr(), m_word_length};
    }
    return {};
}

std::string Token::suffix_to_string()const
{
    if( has_buffer() )
    {
        return std::string{suffix_ptr(), suffix_size()};
    }
    return {};
}

std::string Token::buffer_to_string() const
{
    if (has_buffer())
    {
        return {string_ptr(), m_string_length };
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

    m_buffer                 = other.m_buffer;
    m_word_length            = other.m_word_length;
    m_string_length          = other.m_string_length;
    m_is_buffer_owned        = other.m_is_buffer_owned;
    m_word_start_pos         = other.m_word_start_pos;
    m_string_start_pos       = other.m_string_start_pos;
    m_type                   = other.m_type;
    m_index                  = other.m_index;

    other.m_buffer           = nullptr;
    other.m_string_length    = 0;
    other.m_word_length      = 0;
    other.m_is_buffer_owned  = false;
    other.m_string_start_pos = 0;
    other.m_word_start_pos   = 0;
    other.m_string_start_pos = 0;
    other.m_type             = Token_t::null;
    other.m_index            = 0;

    return *this;
};

Token& Token::operator=(const Token& other)
{
    if( this == &other) return *this;

    if( m_is_buffer_owned )
    {
        delete[] this->m_buffer;
    }

    m_index              = other.m_index;
    m_string_start_pos   = other.m_string_start_pos;
    m_string_length      = other.m_string_length;
    m_word_start_pos     = other.m_word_start_pos;
    m_word_length        = other.m_word_length;
    m_type               = other.m_type;
    m_is_buffer_owned    = other.m_is_buffer_owned;

    if( other.m_is_buffer_owned )
    {
        m_buffer = new char[m_string_length];
        memcpy(m_buffer, other.m_buffer, m_string_length);
    }
    else
    {
        m_buffer = other.m_buffer;
    }

    return *this;
}

void Token::word_replace(const char* new_word)
{
    // Optimization: when buffer is owned and word has same length, we avoid to reallocate memory
    const size_t new_word_len = strlen(new_word);
    if (m_is_buffer_owned && m_word_length == new_word_len  )
    {
        memcpy(word_ptr(), new_word, new_word_len);
        return;
    }

    //TODO: use logarithmic buffer? (with buffer size > string length)
    const size_t new_string_length    = prefix_size() + new_word_len + suffix_size();
    char*        new_buffer           = new char[new_string_length+1];
    size_t       new_string_start_pos = 0;
    size_t       new_word_start_pos   = prefix_size();

    memcpy(new_buffer                                , prefix_ptr() , prefix_size() );
    memcpy(new_buffer + prefix_size()                , new_word     , new_word_len  );
    memcpy(new_buffer + prefix_size() + new_word_len , suffix_ptr() , suffix_size() );
    new_buffer[new_string_length] = '\0';

    if ( m_is_buffer_owned )
        delete[] m_buffer;

    m_buffer           = new_buffer;
    m_word_length      = new_word_len;
    m_string_length    = new_string_length;
    m_string_start_pos = new_string_start_pos;
    m_word_start_pos   = new_word_start_pos;
    m_is_buffer_owned  = true;
}

void Token::suffix_append(const char* str)
{
    const size_t str_len = strlen(str);
    char*        buf     = new char[m_string_length + str_len];

    memcpy(buf, m_buffer + m_string_start_pos, m_string_length); // copy original buffer
    memcpy(buf + m_string_length, str, str_len); // append str

    if ( m_is_buffer_owned )
    {
        delete[] m_buffer;
    }

    m_buffer           = buf;
    m_is_buffer_owned  = true;
    m_word_start_pos   = m_word_start_pos - m_string_start_pos;
    m_string_start_pos = 0;
    m_string_length    += str_len;
}
