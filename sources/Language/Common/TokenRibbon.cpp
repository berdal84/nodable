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
    Token* newToken = new Token(_type, _string, _charIndex);
    tokens.push_back(newToken);
    return newToken;
}

std::string TokenRibbon::toString()const
{
    std::string result;

    for (auto it = tokens.begin(); it != tokens.end(); it++)
    {
        size_t index = it - tokens.begin();

        // Set a color to identify tokens that are inside current transaction
        if ( !transactionStartTokenIndexes.empty() && index >= transactionStartTokenIndexes.top() && index < currentTokenIndex )
        {
            result.append(YELLOW);
        }

        if ( index == currentTokenIndex )
        {
            result.append(BOLDGREEN);
            result.append((*it)->word);
            result.append(RESET);
        }
        else
        {
            result.append((*it)->word);
        }
    }

    const std::string endOfLine("<end>");

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
    if ( canEat() && peekToken()->type == expectedType )
    {
        return eatToken();
    }
    return nullptr;
}

Token* TokenRibbon::eatToken()
{
    LOG_VERBOSE("Parser", "Eat token (idx %i) %s \n", currentTokenIndex, peekToken()->toString().c_str() );
    return tokens.at(currentTokenIndex++);
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
    return tokens.at(currentTokenIndex);
}

Token *TokenRibbon::getEaten()
{
    return currentTokenIndex == 0 ? nullptr : tokens.at(currentTokenIndex - 1);
}