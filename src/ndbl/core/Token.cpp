#include "Token.h"
#include <cstring>

using namespace ndbl;

const Token Token::s_end_of_line        = {Token_t::ignore, "\n"};
const Token Token::s_end_of_instruction = {Token_t::ignore, ";\n"};

Token::Token(Token&& other)
{
    *this = std::move( other );
};

Token::~Token()
{
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
    std::string word_copy = word_to_string();
    m_buffer.switch_to_intern_buf(length());

    // transfer prefix and suffix to this token, but keep the same word.
    // this operation requires the buffer to be owned

    m_prefix_len       = source->m_prefix_len;
    // m_word_len      = unchanged
    m_suffix_len       = source->m_suffix_len;

    // copy prefix from source
    if( source->m_prefix_len )
    {
        m_buffer.intern_buf->append(source->begin(), source->m_prefix_len);
    }

    // reassign word
    if( word_copy.length() )
    {
        m_buffer.intern_buf->append(word_copy.data(), word_copy.length());
    }

    // copy suffix from source
    if( source->m_suffix_len )
    {
        m_buffer.intern_buf->append(source->suffix(), source->m_suffix_len);
    }

    // Remove prefix and suffix on the source
    source->clear_suffix();
    source->clear_prefix();
}

void Token::clear()
{
    m_index      = 0;
    m_type       = Token_t::none;
    m_prefix_len = 0;
    m_word_len   = 0;
    m_suffix_len = 0;

    m_buffer.delete_intern_buf();
}

void Token::set_external_buffer(char* buffer, size_t offset, size_t size)
{
    // here, we consider that the whole buffer will be into the "word" part, no suffix/prefix.

    m_buffer.delete_intern_buf();

    m_buffer.intern = false;
    m_buffer.extern_buf    = buffer;
    m_buffer.offset      = offset;
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

Token& Token::operator=(Token&& other) noexcept
{
    if( this == &other )
    {
        return *this;
    }

    m_buffer.delete_intern_buf();

    m_buffer     = other.m_buffer;
    m_prefix_len = other.m_prefix_len;
    m_word_len   = other.m_word_len;
    m_suffix_len = other.m_suffix_len;
    m_type       = other.m_type;
    m_index      = other.m_index;

    other.m_buffer     = {}; // otherwise it would destroy it

    return *this;
};

Token& Token::operator=(const Token& other)
{
    if( this == &other) return *this;

    m_buffer.delete_intern_buf();

    m_index      = other.m_index;
    m_prefix_len = other.m_prefix_len;
    m_word_len   = other.m_word_len;
    m_suffix_len = other.m_suffix_len;
    m_type       = other.m_type;

    if( other.m_buffer.intern )
    {
        m_buffer.switch_to_intern_buf(other.length());
        m_buffer.intern_buf->append(other.begin(), other.length() );
    }
    else
    {
        m_buffer = other.m_buffer;
    }

    return *this;
}

void Token::word_replace(const char* new_word)
{
    const size_t new_word_len = strlen(new_word);

    if( new_word_len == 0 )
        if( length() == 0 )
            return;

    // Optimization: when buffer is owned and word has same length, we avoid to reallocate memory
    if (m_buffer.intern && m_word_len == new_word_len  )
    {
        m_buffer.intern_buf->replace(m_prefix_len, m_word_len, new_word, new_word_len);
        return;
    }

    std::string prefix_copy = prefix_to_string();
    std::string suffix_copy = suffix_to_string();

    m_buffer.switch_to_intern_buf(m_prefix_len + new_word_len + m_suffix_len);

    m_buffer.intern_buf->clear();
    m_buffer.intern_buf->append(prefix_copy );
    m_buffer.intern_buf->append(new_word );
    m_buffer.intern_buf->append(suffix_copy );

    // m_prefix_len    = no change
    m_word_len         = new_word_len;
    // m_suffix_len    = no change
}

void Token::prefix_push_front(const char* str)
{
    const size_t str_len = strlen(str);
    m_buffer.switch_to_intern_buf_with_data(begin(), length());
    m_buffer.intern_buf->insert(0, str);
    m_prefix_len += str_len;
}

void Token::suffix_push_back(const char* str)
{
    const size_t str_len = strlen(str);
    m_buffer.switch_to_intern_buf_with_data(begin(), length());
    m_buffer.intern_buf->append(str);
    m_suffix_len += str_len;
}

void Token::clear_prefix()
{
    // Instead of erasing chars, we prefer to simply "move the cursor to the right"
    m_buffer.offset += m_prefix_len;
    m_prefix_len  = 0;
}

void Token::clear_suffix()
{
    // Instead of erasing chars, we prefer to simply "move the cursor to the left"
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

void Token::set_offset(size_t pos)
{
    VERIFY(!m_buffer.intern, "This method is only allowed when buffer is external");
    m_buffer.offset = pos;
}

void Token::resize_suffix(size_t size)
{
    VERIFY(!m_buffer.intern, "This method is only allowed when buffer is external");
    m_suffix_len = size;
}

void Token::extend_prefix(size_t size)
{
    VERIFY(!m_buffer.intern, "This method is only allowed when buffer is external");
    m_prefix_len += size;
}

void Token::extend_suffix(size_t size)
{
    VERIFY(!m_buffer.intern, "This method is only allowed when buffer is external");
    resize_suffix( m_suffix_len + size);
}

Token::BimodalBuffer::~BimodalBuffer()
{
    delete_intern_buf();
}

void Token::BimodalBuffer::switch_to_intern_buf(size_t size)
{
    ASSERT( size != 0 ); // Why would you do that?

    // Initialize memory
    if ( !intern )
    {
        intern_buf = new std::string();
        intern     = true;
        offset     = 0;
    }
    intern_buf->reserve(size);
}

void Token::BimodalBuffer::switch_to_intern_buf_with_data(char* data, size_t size)
{
    switch_to_intern_buf(size);
    intern_buf->append( data, size );
}


void Token::BimodalBuffer::delete_intern_buf()
{
    if( !intern )
        return;

    delete intern_buf;

    intern     = false;
    offset     = 0;
    intern_buf = nullptr;
}