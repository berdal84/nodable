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
    out.append(RESET);

    size_t buffer_size = 0;

    // get the total buffer sizes (but won't be exact, some token are serialized dynamically)
    for (const Token& each_token : m_tokens)
        buffer_size += each_token.length();

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
            line.append(each_token.prefix(), each_token.prefix_len());

        line.append("\", \"");

        if (each_token.has_buffer())
            line.append(each_token.word(), each_token.word_len());

        line.append("\", \"");

        if (each_token.has_buffer())
            line.append(each_token.suffix(), each_token.suffix_len());

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
    return Token_t::none;
}

Token TokenRibbon::eat()
{
    LOG_VERBOSE("TokenRibbon", "Eat token (idx %i) %s \n", m_cursor, peek().string().c_str() );
    return m_tokens.at(m_cursor++);
}

void TokenRibbon::start_transaction()
{
    m_transaction.push(m_cursor);
    LOG_VERBOSE("TokenRibbon", "Start Transaction (idx %i)\n", m_cursor);
}

void TokenRibbon::rollback()
{
    m_cursor = m_transaction.top();
    LOG_VERBOSE("TokenRibbon", "Rollback (idx %i)\n", m_cursor);
    m_transaction.pop();
}

void TokenRibbon::commit()
{
    LOG_VERBOSE("TokenRibbon", "Commit (idx %i)\n", m_cursor);
    m_transaction.pop();
}

void TokenRibbon::reset(const char* new_buffer, size_t new_size)
{
    auto buffer = const_cast<char*>(new_buffer);

    m_tokens.clear();

    m_global_token.set_external_buffer( buffer, 0, new_size, true ); // wraps all

    while(!m_transaction.empty())
        m_transaction.pop();

    m_cursor = 0;
}

bool TokenRibbon::can_eat(size_t count) const
{
    ASSERT(count > 0);
    return m_cursor + count <= m_tokens.size() ;
}

std::string TokenRibbon::range_to_string(size_t pos, int size)
{
    ASSERT( size != 0);
    ASSERT( pos < m_tokens.size() );

    // ensure size is positive
    if( size < 0 )
    {
        ASSERT( -size < m_tokens.size() - pos );
        pos  = pos + size;
        size = -size;
    }
    else
    {
        ASSERT( pos + size < m_tokens.size() );
    }

    std::string result;
    for( size_t i = pos; i < pos + size; ++i )
        result = result + m_tokens[pos].string();
    return result;
}
