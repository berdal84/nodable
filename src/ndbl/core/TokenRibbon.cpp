#include "TokenRibbon.h"

#include "tools/core/log.h"
#include "tools/core/assertions.h"

#include "Token_t.h"
#include "Token.h"

using namespace ndbl;

Token & TokenRibbon::push(Token &_token)
{
    _token.m_index = m_tokens.size();
    m_tokens.push_back(_token);
    return m_tokens.back();
}

std::string TokenRibbon::to_string()const
{
    std::string out;
    size_t buffer_size = 0;

    // get the total buffer sizes (but won't be exact, some token are serialized dynamically)
    for (const Token& each_token : m_tokens)
        buffer_size += each_token.m_string_length;

    out.reserve(buffer_size);
    out.append("[<TokenRibbon start>]\n");

    for (const Token& each_token : m_tokens)
    {
        tools::string256 line;
        size_t index = each_token.m_index;

        // Set a color to identify tokens that are inside current transaction
        if (!m_transaction.empty() && index >= m_transaction.top() && index < m_cursor )
        {
            line.append(YELLOW);
        }

        if (index == m_cursor )
            line.append(BOLDGREEN);

        line.append_fmt("[%4llu]", index);

        line.append("[\"");

        if (each_token.has_buffer())
            line.append(each_token.prefix_ptr(), each_token.prefix_size());

        line.append("\", \"");

        if (each_token.has_buffer())
            line.append(each_token.word_ptr(), each_token.word_size());

        line.append("\", \"");

        if (each_token.has_buffer())
            line.append(each_token.suffix_ptr(), each_token.suffix_size());

        line.append("\"]");

        if (index == m_cursor )
            line.append(RESET);

        line.push_back('\n');

        out.append(line.c_str() );
    }

    if (m_tokens.size() == m_cursor )
        out.append(GREEN);
    out.append("[<TokenRibbon end>]");
    out.append(RESET"\n");

    return out;
}

Token TokenRibbon::eat_if(Token_t expectedType)
{
    if (can_eat() && peek().m_type == expectedType )
    {
        return eat();
    }
    return Token::s_null;
}

Token TokenRibbon::eat()
{
    LOG_VERBOSE("Parser", "Eat token (idx %i) %s \n", m_cursor, peek().buffer_to_string().c_str() );
    return m_tokens.at(m_cursor++);
}

void TokenRibbon::transaction_start()
{
    m_transaction.push(m_cursor);
    LOG_VERBOSE("Parser", "Start Transaction (idx %i)\n", m_cursor);
}

void TokenRibbon::transaction_rollback()
{
    m_cursor = m_transaction.top();
    LOG_VERBOSE("Parser", "Rollback transaction (idx %i)\n", m_cursor);
    m_transaction.pop();
}

void TokenRibbon::transaction_commit()
{
    LOG_VERBOSE("Parser", "Commit transaction (idx %i)\n", m_cursor);
    m_transaction.pop();
}

void TokenRibbon::clear()
{
    m_tokens.clear();
    m_prefix.m_type             = m_suffix.m_type             = Token_t::ignore;
    m_prefix.m_string_start_pos = m_suffix.m_string_start_pos = 0;
    m_prefix.m_string_length      = m_suffix.m_string_length      = 0;
    m_prefix.m_word_start_pos   = m_suffix.m_word_start_pos   = 0;
    m_prefix.m_word_length        = m_suffix.m_word_length        = 0;
    while(!m_transaction.empty())
    {
        m_transaction.pop();
    }
    m_cursor = 0;
}

void TokenRibbon::set_source_buffer(const char* const_buffer)
{
    auto buffer = const_cast<char*>(const_buffer);
    m_prefix.set_source_buffer(buffer);
    m_suffix.set_source_buffer(buffer);
}

bool TokenRibbon::can_eat(size_t count) const
{
    ASSERT(count > 0);
    return m_cursor + count <= m_tokens.size() ;
}

std::string TokenRibbon::concat_token_buffers(size_t pos, int size)
{
    std::string result;
    size_t idx = pos;
    int step = size > 0 ? 1 : -1;
    int step_count = step * size;
    int step_done_count = 0;
    while( idx > 0 && idx < m_tokens.size() && step_done_count <= step_count )
    {
        Token* token = &m_tokens[idx];
        result = step > 0 ? result + token->buffer_to_string() : token->buffer_to_string() + result;

        idx += step;
        step_done_count++;
    }

    return result;
}
