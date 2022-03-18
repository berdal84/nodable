#include <nodable/core/Token.h>

using namespace Nodable;

const std::shared_ptr<Token> Token::s_null = std::make_shared<Token>(TokenType_NULL);

std::string Token::to_string(std::shared_ptr<Token> _token)
{
    std::string result;
    result.append("{ ");
    result.append( "word: " + _token->m_word );
    result.append( ", charIndex: " + std::to_string(_token->m_charIndex) );
    result.append(" }");
    return result;
}