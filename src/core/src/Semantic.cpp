#include "Semantic.h"
#include "Log.h"

using namespace Nodable;

Semantic::Semantic()
{
    m_typeToTokenType.resize(Type_COUNT);
    m_tokenTypeToType.resize(TokenType_COUNT);
    m_tokenTypeToString.resize(TokenType_COUNT);
}

void Semantic::insert(const std::regex& _regex, TokenType _tokenType)
{
    m_regex.push_back(_regex);
    m_regexIndexToTokenType.push_back(_tokenType);
}

void Semantic::insert(std::string _string, TokenType _tokenType)
{
    m_tokenTypeToString[_tokenType] = _string;

    // Clean string before to create a regex
    // TODO: improve it to solve all regex escape char problems
    if (_string == ")" || _string == "(" || _string == "}" || _string == "{") {
        _string.insert(_string.begin(), '[');
        _string.insert(_string.end()  , ']');
    }

    _string.insert(_string.begin(), '(');
    _string.insert(_string.end()  , ')');

    insert(std::regex("^" + _string), _tokenType);
}

void Semantic::insert(Type _type, TokenType _tokenType)
{
    m_tokenTypeToType[_tokenType] = _type;
    m_typeToTokenType[_type] = _tokenType;
}
