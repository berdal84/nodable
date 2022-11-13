#include <nodable/core/TokenRibbon.h>

#include <nodable/core/Token_t.h>
#include <nodable/core/Token.h>
#include <nodable/core/Log.h>
#include <nodable/core/assertions.h>

using namespace ndbl;

TokenRibbon::TokenRibbon()
    : m_curr_tok_idx(0)
    , m_prefix( std::make_shared<Token>() )
    , m_suffix( std::make_shared<Token>() )
{
    transactionStartTokenIndexes.push(0);
}

s_ptr<Token> TokenRibbon::push(s_ptr<Token> _token)
{
    _token->m_index = tokens.size();
    tokens.push_back(_token);
    return tokens.back();
}

s_ptr<Token> TokenRibbon::push(Token_t  _type, const std::string& _string, size_t _charIndex )
{
    s_ptr<Token> token = std::make_shared<Token>(_type, _string, _charIndex);
    token->m_index = tokens.size();
    tokens.push_back(token);
    return tokens.back();
}

std::string TokenRibbon::toString()const
{
    // TODO: optimization: split in 3 loops (before current transaction, current transaction range, after transaction range)
    //       to avoid those if in loops.
    std::string result;
    result.append("[[ ");
    for (auto eachTokIt = tokens.begin(); eachTokIt != tokens.end(); eachTokIt++)
    {
        size_t index = eachTokIt - tokens.begin();

        // Set a color to identify tokens that are inside current transaction
        if ( !transactionStartTokenIndexes.empty() && index >= transactionStartTokenIndexes.top() && index < m_curr_tok_idx )
        {
            result.append(YELLOW);
        }

        if (index == m_curr_tok_idx )
        {
            result.append("> ");
            result.append(BOLDGREEN);
            result.append((*eachTokIt)->m_word);
            result.append(RESET);
            result.append(" <");
        }
        else
        {
            result.append((*eachTokIt)->m_word);
        }

        if ( tokens.end() != eachTokIt )
            result.append("|");
    }

    const std::string endOfLine("<eol>");

    if (tokens.size() == m_curr_tok_idx )
    {
        result.append(GREEN);
        result.append(endOfLine);
    }
    else
    {
        result.append(endOfLine);
    }

    result.append(" ]]");
    result.append(RESET);

    return result;
}

s_ptr<Token> TokenRibbon::eatToken(Token_t expectedType)
{
    if ( canEat() && peekToken()->m_type == expectedType )
    {
        return eatToken();
    }
    return nullptr;
}

s_ptr<Token> TokenRibbon::eatToken()
{
    LOG_VERBOSE("Parser", "Eat token (idx %i) %s \n", m_curr_tok_idx, Token::to_string(peekToken()).c_str() )
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
    m_prefix->clear();
    m_suffix->clear();
    transactionStartTokenIndexes = std::stack<size_t>();
    m_curr_tok_idx = 0;
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
    NODABLE_ASSERT(_tokenCount > 0);
    return m_curr_tok_idx + _tokenCount <= tokens.size() ;
}

s_ptr<Token> TokenRibbon::peekToken()
{
    return tokens.at(m_curr_tok_idx);
}

s_ptr<Token> TokenRibbon::getEaten()
{
    // TODO: optimization: store a pointer to the last eaten Token ?
    return m_curr_tok_idx == 0 ? nullptr : tokens.at(m_curr_tok_idx - 1);
}