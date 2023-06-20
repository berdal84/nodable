#include <ndbl/core/TokenRibbon.h>

#include "fw/core/log.h"
#include "fw/core/assertions.h"

#include <ndbl/core/Token_t.h>
#include <ndbl/core/Token.h>

using namespace ndbl;

TokenRibbon::TokenRibbon()
    : m_curr_tok_idx(0)
{
    transactionStartTokenIndexes.push(0);
}

Token & TokenRibbon::push(Token &_token)
{
    _token.m_index = tokens.size();
    tokens.push_back(_token);
    return tokens.back();
}

std::string TokenRibbon::to_string()const
{
    std::string out_buffer;
    size_t buffer_size = 0;

    // get the total buffer sizes (but won't be exact, some token are serialized dynamically)
    for (const Token& each_token : tokens)
    {
        buffer_size += each_token.m_buffer_size;
    }
    out_buffer.reserve(buffer_size);

    out_buffer.append("[<begin>]=");
    for (const Token& each_token : tokens)
    {
        size_t index = each_token.m_index;

        // Set a color to identify tokens that are inside current transaction
        if ( !transactionStartTokenIndexes.empty() && index >= transactionStartTokenIndexes.top() && index < m_curr_tok_idx )
        {
            out_buffer.append(YELLOW);
        }

        if (index == m_curr_tok_idx ) out_buffer.append(BOLDGREEN);
        out_buffer.append("[\"");
        if (each_token.has_buffer()) out_buffer.append(each_token.buffer(), each_token.prefix_size());
        out_buffer.append("\", \"");
        if (each_token.has_buffer()) out_buffer.append(each_token.word(), each_token.word_size());
        out_buffer.append("\", \"");
        if (each_token.has_buffer()) out_buffer.append(each_token.suffix(), each_token.suffix_size());
        out_buffer.append("\"]");
        if (index == m_curr_tok_idx )    out_buffer.append(RESET);
        out_buffer.append("=");
    }

    if (tokens.size() == m_curr_tok_idx ) out_buffer.append(GREEN);
    out_buffer.append("[<eol>]");
    out_buffer.append(RESET);

    return out_buffer;
}

Token TokenRibbon::eatToken(Token_t expectedType)
{
    if ( canEat() && peekToken().m_type == expectedType )
    {
        return eatToken();
    }
    return Token::s_null;
}

Token TokenRibbon::eatToken()
{
    LOG_VERBOSE("Parser", "Eat token (idx %i) %s \n", m_curr_tok_idx, peekToken().buffer_to_string().c_str() )
    return tokens.at(m_curr_tok_idx++);
}

void TokenRibbon::startTransaction()
{
    transactionStartTokenIndexes.push(m_curr_tok_idx);
    LOG_VERBOSE("Parser", "Start Transaction (idx %i)\n", m_curr_tok_idx)
}

void TokenRibbon::rollbackTransaction()
{
    m_curr_tok_idx = transactionStartTokenIndexes.top();
    LOG_VERBOSE("Parser", "Rollback transaction (idx %i)\n", m_curr_tok_idx)
    transactionStartTokenIndexes.pop();
}

void TokenRibbon::commitTransaction()
{
    LOG_VERBOSE("Parser", "Commit transaction (idx %i)\n", m_curr_tok_idx)
    transactionStartTokenIndexes.pop();
}

void TokenRibbon::clear()
{
    tokens.clear();
    m_prefix_acc = {Token_t::ignore};
    m_suffix_acc = {Token_t::ignore};
    transactionStartTokenIndexes = std::stack<size_t>();
    m_curr_tok_idx = 0;
}

void TokenRibbon::set_source_buffer(const std::string &_buffer)
{
    m_source_buffer = _buffer;
    m_prefix_acc.set_source_buffer(m_source_buffer.data());
    m_suffix_acc.set_source_buffer(m_source_buffer.data());
}

bool TokenRibbon::empty() const
{
    return tokens.empty();
}

size_t TokenRibbon::size() const
{
    return tokens.size();
}

bool TokenRibbon::canEat(size_t _tokenCount) const
{
    FW_ASSERT(_tokenCount > 0);
    return m_curr_tok_idx + _tokenCount <= tokens.size() ;
}

const Token& TokenRibbon::peekToken() const
{
    return tokens.at(m_curr_tok_idx);
}

const Token& TokenRibbon::getEaten()const
{
    // TODO: optimization: store a pointer to the last eaten Token ?
    return m_curr_tok_idx == 0 ? Token::s_null : tokens.at(m_curr_tok_idx - 1);
}

std::string TokenRibbon::concat_token_buffers(size_t offset, int count)
{
    std::string result;
    size_t idx = offset;
    int step = count > 0 ? 1 : -1;
    int step_count = step * count;
    int step_done_count = 0;
    while( idx > 0 && idx < tokens.size() && step_done_count <= step_count )
    {
        Token* token = &tokens[idx];
        result = step > 0 ? result + token->buffer_to_string() : token->buffer_to_string() + result;

        idx += step;
        step_done_count++;
    }

    return result;
}
