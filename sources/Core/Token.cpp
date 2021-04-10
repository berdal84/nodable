#include "Token.h"

using namespace Nodable;

const Token Token::Null = Token(TokenType_NULL);

bool Token::isOperand(TokenType type)
{
    return 0 != (type & TokenType_Double | TokenType_Boolean | TokenType_String | TokenType_Symbol);
}

std::string Token::toString(const Token* _token)
{
    std::string result;
    result.append("{ ");
    result.append( "word: " + _token->word );
    result.append( ", charIndex: " + std::to_string(_token->charIndex) );
    result.append(" }");
    return result;
}