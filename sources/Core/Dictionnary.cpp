#include "Dictionnary.h"

using namespace Nodable;

std::string Dictionnary::convert(const TokenType& _type)const
{
    return tokenTypeToString.at(_type);
}

void Dictionnary::add(
    std::regex _regex,
    TokenType _tokenType)
{
    NODABLE_ASSERT(
        tokenTypeToRegex.find(_tokenType) == tokenTypeToRegex.end(),
        "Unable to add another regex, this token alread has one.");

    tokenTypeToRegex[_tokenType] = _regex;
}

void Dictionnary::add(
    std::string _string,
    TokenType _tokenType)
{
    // Clean string to create a regex:
    if (_string.size() == 1) {
        _string.insert(_string.begin(), '[');
        _string.insert(_string.end()  , ']');
    }

    add(std::regex("^" + _string), _tokenType);
    tokenTypeToString[_tokenType] = _string;
}

void Dictionnary::add(
    std::string _string,
    TokenType _tokenType,
    std::regex _regex)
{

    add(_regex, _tokenType);
    tokenTypeToString[_tokenType] = _string;
}
