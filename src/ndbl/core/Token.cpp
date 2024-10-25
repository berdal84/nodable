#include "Token.h"
#include <cstring>

using namespace ndbl;

const Token Token::s_end_of_line        = {Token_t::ignore, "\n"};
const Token Token::s_end_of_instruction = {Token_t::ignore, ";\n"};

Token::~Token()
{
    if( m_is_buffer_owned )
        delete[] m_buffer;
}

std::string Token::json() const
{
    std::string result;
    result.reserve( length() );

    result.append("{ ");

    result.append("word: \"" );
    result.append(word(), word_len());
    result.append("\"" );

    result.append(", charIndex: " + std::to_string( (u64_t)word() - (u64_t) begin() ) );

    result.append(", prefix: \""  );
    result.append(prefix(), prefix_len());
    result.append("\""  );

    result.append(", suffix: \""  );
    result.append(suffix(), suffix_len());
    result.append("\""  );

    result.append(", token_type: \"" + std::to_string((int)m_type) + "\"");

    result.append(" }");

    return result;
}

void Token::take_prefix_suffix_from(Token* source)
{
    if( m_is_buffer_owned )
        delete[] m_buffer;

    std::string word_copy;
    if ( m_buffer != nullptr )
        word_copy.append( word(), word_len() );

    // transfer prefix and suffix to this token, but keep the same word.
    // this operation requires the buffer to be owned

    m_buffer           = new char[ length() ];
    m_is_buffer_owned  = true;
    m_data_pos         = 0;
    m_prefix_len       = source->m_prefix_len;
    // m_word_len      = unchanged
    m_suffix_len       = source->m_suffix_len;

    // copy prefix from source
    if( source->m_prefix_len )
    {
        memcpy( begin(), source->begin(), source->m_prefix_len);
    }

    // reassign word
    if( word_copy.length() )
    {
        memcpy(word(), word_copy.data(), word_copy.length());
    }

    // copy suffix from source
    if( source->m_suffix_len )
    {
        memcpy(suffix(), source->suffix(), source->m_suffix_len);
    }

    // Remove prefix and suffix on the source
    source->clear_suffix();
    source->clear_prefix();
}

void Token::clear()
{
    m_index      = 0;
    m_type       = Token_t::none;
    m_data_pos   = 0;
    m_prefix_len = 0;
    m_word_len   = 0;
    m_suffix_len = 0;

    if( m_is_buffer_owned )
    {
        delete[] m_buffer;
        m_buffer = nullptr;
    }
}

void Token::set_source_buffer(char *_buffer, size_t pos, size_t size)
{
    // here, we consider that the whole buffer will be into the "word" part, no suffix/prefix.

    if( m_is_buffer_owned )
        delete[] m_buffer;

    m_buffer     = _buffer;
    m_data_pos   = pos;
    m_prefix_len = 0;
    m_word_len   = size;
    m_suffix_len = 0;
}

std::string Token::prefix_to_string()const
{
    if( has_buffer() )
    {
        return std::string{prefix(), m_prefix_len};
    }
    return {};
}

std::string Token::word_to_string()const
{
    if( has_buffer() )
        return std::string{word(), m_word_len};
    return {};
}

std::string Token::suffix_to_string()const
{
    if( has_buffer() )
        return std::string{suffix(), m_suffix_len};
    return {};
}

std::string Token::string() const
{
    if ( has_buffer() )
        return { begin(), length() };
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
    m_data_pos               = other.m_data_pos;
    m_is_buffer_owned        = other.m_is_buffer_owned;
    m_prefix_len             = other.m_prefix_len;
    m_word_len               = other.m_word_len;
    m_suffix_len             = other.m_suffix_len;
    m_type                   = other.m_type;
    m_index                  = other.m_index;

    other.m_buffer           = nullptr;
    other.m_is_buffer_owned  = false;
    other.clear();

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
    m_data_pos           = other.m_data_pos;
    m_prefix_len         = other.m_prefix_len;
    m_word_len           = other.m_word_len;
    m_suffix_len         = other.m_suffix_len;
    m_type               = other.m_type;
    m_is_buffer_owned    = other.m_is_buffer_owned;

    if( other.m_is_buffer_owned )
    {
        const size_t new_length = other.length();
        m_buffer = new char[ new_length ];
        memcpy(m_buffer, other.m_buffer, new_length);
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
    if ( m_is_buffer_owned && word_len() == new_word_len  )
    {
        memcpy(word(), new_word, new_word_len);
        return;
    }

    //TODO: use logarithmic buffer? (with buffer size > string length)
    const size_t new_length = m_prefix_len + new_word_len + m_suffix_len;
    char*        new_buffer = new char[ new_length ]; // We do not terminate strings with NULL.

    memcpy(new_buffer                               , prefix() , m_prefix_len);
    memcpy(new_buffer + prefix_len()                , new_word , new_word_len  );
    memcpy(new_buffer + prefix_len() + new_word_len , suffix() , m_suffix_len );

    if ( m_is_buffer_owned )
        delete[] m_buffer;

    m_data_pos         = 0;
    m_buffer           = new_buffer;
    m_is_buffer_owned  = true;
    // m_prefix_len    = no change
    m_word_len         = new_word_len;
    // m_suffix_len    = no change
}

void Token::suffix_append(const char* str)
{
    const size_t str_len = strlen(str);
    char*        buf     = new char[length() + str_len];

    memcpy(buf           , begin(), length() ); // copy original data
    memcpy(buf + length(), str    , str_len);   // append str

    if ( m_is_buffer_owned )
    {
        delete[] m_buffer;
    }

    m_data_pos         = 0;
    m_buffer           = buf;
    m_is_buffer_owned  = true;
    m_suffix_len      += str_len;
}

void Token::clear_prefix()
{
    // no matter if buffer is owned or not, we simply slide the data_position inside the string
    m_data_pos   += m_prefix_len;
    m_prefix_len  = 0;
}

void Token::clear_suffix()
{
    // no matter if buffer is owned or not, we simply slide the data_position inside the string
    m_suffix_len  = 0;
}

void Token::reset_lengths()
{
    m_prefix_len = m_word_len = m_suffix_len = 0;
}

void Token::slide_word_begin(int amount)
{
    ASSERT( amount < 0 ? -amount < m_prefix_len : amount <= m_word_len);

    m_prefix_len = (int)m_prefix_len + amount;
    m_word_len   = (int)m_word_len - amount;
}

void Token::slide_word_end(int amount)
{
    ASSERT( amount < 0 ? -amount < m_word_len : amount <= m_suffix_len);

    m_word_len   = (int)m_word_len + amount;
    m_suffix_len = (int)m_suffix_len - amount;
}