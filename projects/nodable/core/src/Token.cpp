#include "fw/core/assertions.h"

#include <ndbl/core/Token.h>

using namespace ndbl;

const Token Token::s_null(Token_t::null);

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

void Token::transfer_prefix_and_suffix_from(Token* source)
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
        strncpy(m_source_buffer, source->buffer(), prefix_size);
        m_word_start_pos = prefix_size;
    }

    // reassign word
    if( m_word_size )
    {
        strncpy(word(), word_copy.data(), m_word_size);
    }

    // copy suffix from source
    if( size_t suffix_size = source->suffix_size())
    {
        strncpy(suffix(), source->suffix(), suffix_size);
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
