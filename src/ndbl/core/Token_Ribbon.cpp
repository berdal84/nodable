#include "Token_Ribbon.h"

#include "tools/core/Log.h"
#include "tools/core/Asserts.h"

#include "Token_Type.h"
#include "Token.h"

using namespace ndbl;

Token & Token_Ribbon::push(Token &_token)
{
    _token.m_index = m_tokens.size();
    m_tokens.push_back(_token);
    return m_tokens.back();
}

std::string Token_Ribbon::to_string()const
{
    tools::String out;
    out.append(TOOLS_COLOR_DEFAULT);

    size_t buffer_size = 0;

    // get the total buffer sizes (but won't be exact, some token are serialized dynamically)
    for (const Token& each_token : m_tokens)
        buffer_size += each_token.length();

    out.append("Logging token ribbon state:\n");
    out.append("___________[TOKEN RIBBON]_________\n");

    for (const Token& token : m_tokens)
    {
        tools::String_512 line;
        
        if ( token.m_index == 0 )
        {
            line.append("B"); // begin
        }
        else if ( token.m_index == m_tokens.back().m_index )
        {
            line.append("E"); // end
        }
        else
        {
            line.append("|"); // default
        }

        if ( !m_transaction.empty()
              && token.m_index >= m_transaction.top()
              && token.m_index <= m_cursor )
        {
            line.append("T"); // transaction
        }
        else
        {
            line.append("."); // no transaction
        }

        const std::string word = token.word_to_string();
        line.append_fmt("%5zu) \"%s\"", token.m_index, word.c_str() );
      
        if ( token.m_index == m_cursor )
        {
            line.append(" [c]"); // current
        }
        
        out.append(line.c_str());
        out.append("\n");
    }

    return out.c_str();
}

Token Token_Ribbon::eat_if(Token_Type expectedType)
{
    if (can_eat() && peek().m_type == expectedType )
    {
        return eat();
    }
    return Token_Type::none;
}

Token Token_Ribbon::eat()
{
    TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Token_Ribbon", "Eat token (idx %i) %s \n", m_cursor, peek().string().c_str() );
    return m_tokens.at(m_cursor++);
}

void Token_Ribbon::start_transaction()
{
    m_transaction.push(m_cursor);
    TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Token_Ribbon", "Start Transaction (idx %i)\n", m_cursor);
}

void Token_Ribbon::rollback()
{
    m_cursor = m_transaction.top();
    TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Token_Ribbon", "Rollback (idx %i)\n", m_cursor);
    m_transaction.pop();
}

void Token_Ribbon::commit()
{
    TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Token_Ribbon", "Commit (idx %i)\n", m_cursor);
    m_transaction.pop();
}

void Token_Ribbon::reset(const char* new_buffer, size_t new_size)
{
    auto buffer = const_cast<char*>(new_buffer);

    m_tokens.clear();

    m_global_token.set_external_buffer( buffer, 0, new_size, true ); // wraps all

    while(!m_transaction.empty())
        m_transaction.pop();

    m_cursor = 0;
}

bool Token_Ribbon::can_eat(size_t count) const
{
    ASSERT(count > 0);
    return m_cursor + count <= m_tokens.size() ;
}

std::string Token_Ribbon::range_to_string(size_t begin, size_t end)
{
    ASSERT(begin <= end);
    ASSERT(end <= m_tokens.size() );

    std::string result;
    for( size_t i = begin; i < end; ++i )
    {
        result.append( m_tokens[i].string() );
    }
    return result;
}
