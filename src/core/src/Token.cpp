#include <nodable/core/Token.h>
#include <nodable/core/assertions.h>

using namespace ndbl;

const std::shared_ptr<Token> Token::s_null = std::make_shared<Token>(Token_t::null);

std::string Token::to_JSON(const std::shared_ptr<Token> _token)
{
    std::string result;
    result.append("{ ");
    result.append( "word: \"" + _token->get_word() + "\"" );
    result.append( ", charIndex: " + std::to_string(_token->m_charIndex) );
    result.append( ", prefix: \"" + _token->get_prefix() + "\""  );
    result.append( ", suffix: \"" + _token->get_suffix() + "\""  );
    result.append( ", buffer: \"" + _token->m_buffer + "\""  );
    result.append(" }");
    return result;
}

std::string Token::to_string(const std::shared_ptr<Token> _token)
{
    return _token->m_buffer;
}

void Token::append_to_prefix(const std::string& str)
{
    m_buffer.insert( m_word_pos, str);
    m_word_pos += str.size();
}

void Token::append_to_suffix(const std::string& str)
{
    m_buffer += str;
}

void Token::append_to_word(const std::string& str)
{
    m_buffer.insert(m_word_pos + m_word_size, str);
    m_word_size += str.size();
}

void Token::append_to_word(char c)
{
    m_buffer.insert( m_word_pos + m_word_size, 1, c );
    m_word_size += 1;
}

std::string Token::get_prefix() const
{
    return m_buffer.substr(0, m_word_pos);
}

std::string Token::get_suffix() const
{
    return m_buffer.substr(m_word_pos + m_word_size);
}

std::string Token::get_word() const
{
    return m_buffer.substr(m_word_pos, m_word_size);
}

void Token::transfer_prefix_suffix(const std::shared_ptr<Token> other)
{
    // transfer from "other" to "this"
    auto word_backup = get_word();
    m_buffer    = "";
    m_word_pos  = 0;
    m_word_size = 0;
    append_to_prefix(other->get_prefix());
    append_to_word(word_backup);
    append_to_suffix(other->get_suffix());

    // cleanup "other"
    other->m_buffer    = other->get_word();
    other->m_word_pos  = 0;
    other->m_word_size = other->m_buffer.size();
}

void Token::clear()
{
    m_index     = 0;
    m_type      = Token_t::default_;
    m_charIndex = 0;
    m_word_pos  = 0;
    m_word_size = 0;
    m_buffer    = "";
}
