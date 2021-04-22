#include "Token.h"

using namespace Nodable;

const Token Token::Null = Token(TokenType_NULL);

std::string Token::toString(const Token* _token)
{
    std::string result;
    result.append("{ ");
    result.append( "word: " + _token->word );
    result.append( ", charIndex: " + std::to_string(_token->charIndex) );
    result.append(" }");
    return result;
}