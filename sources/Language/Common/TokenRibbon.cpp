#include <Nodable.h>
#include "TokenRibbon.h"
#include "TokenType.h"
#include "Token.h"
#include "Log.h"

using namespace Nodable;

TokenRibbon::TokenRibbon()
    :
    currentTokenIndex(0)
{
    transactionStartTokenIndexes.push(0);
}

Token* TokenRibbon::push(TokenType  _type, const std::string& _string, size_t _charIndex )
{
    return &tokens.emplace_back(_type, _string, _charIndex);
}

std::string TokenRibbon::toString()const
{
    // TODO: optimization: split in 3 loops (before current transaction, current transaction range, after transaction range)
    //       to avoid those if in loops.
    std::string result;

    for (auto eachTokIt = tokens.begin(); eachTokIt != tokens.end(); eachTokIt++)
    {
        size_t index = eachTokIt - tokens.begin();

        // Set a color to identify tokens that are inside current transaction
        if ( !transactionStartTokenIndexes.empty() && index >= transactionStartTokenIndexes.top() && index < currentTokenIndex )
        {
            result.append(YELLOW);
        }

        if ( index == currentTokenIndex )
        {
            result.append(BOLDGREEN);
            result.append((*eachTokIt).m_word);
            result.append(RESET);
        }
        else
        {
            result.append((*eachTokIt).m_word);
        }
    }

    const std::string endOfLine("<eol>");

    if (tokens.size() == currentTokenIndex )
    {
        result.append(GREEN);
        result.append(endOfLine);
    }
    else
    {
        result.append(endOfLine);
    }

    result.append(RESET);

    return result;
}

Token* TokenRibbon::eatToken(TokenType expectedType)
{
    if ( canEat() && peekToken()->m_type == expectedType )
    {
        return eatToken();
    }
    return nullptr;
}

Token* TokenRibbon::eatToken()
{
    LOG_VERBOSE("Parser", "Eat token (idx %i) %s \n", currentTokenIndex, Token::toString( peekToken() ).c_str() );
    return &tokens.at(currentTokenIndex++);
}

void TokenRibbon::startTransaction()
{
    transactionStartTokenIndexes.push(currentTokenIndex);
    LOG_VERBOSE("Parser", "Start Transaction (idx %i)\n", currentTokenIndex);
}

void TokenRibbon::rollbackTransaction()
{
    currentTokenIndex = transactionStartTokenIndexes.top();
    LOG_VERBOSE("Parser", "Rollback transaction (idx %i)\n", currentTokenIndex);
    transactionStartTokenIndexes.pop();
}

void TokenRibbon::commitTransaction()
{
    LOG_VERBOSE("Parser", "Commit transaction (idx %i)\n", currentTokenIndex);
    transactionStartTokenIndexes.pop();
}

void TokenRibbon::clear()
{
    tokens.clear();
    transactionStartTokenIndexes = std::stack<size_t>();
    currentTokenIndex = 0;
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
    return  currentTokenIndex + _tokenCount <= tokens.size() ;
}

Token* TokenRibbon::peekToken()
{
    return &tokens.at(currentTokenIndex);
}

Token *TokenRibbon::getEaten()
{
    // TODO: optimization: store a pointer to the last eaten Token ?
    return currentTokenIndex == 0 ? nullptr : &tokens.at(currentTokenIndex - 1);
}