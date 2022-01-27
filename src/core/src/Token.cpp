#include <nodable/Token.h>

using namespace Nodable;

const Token Token::s_null = Token(TokenType_NULL);

std::string Token::to_string(const Token* _token)
{
    std::string result;
    result.append("{ ");
    result.append( "word: " + _token->m_word );
    result.append( ", charIndex: " + std::to_string(_token->m_charIndex) );
    result.append(" }");
    return result;
}