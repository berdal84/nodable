#include "Semantic.h"

using namespace Nodable;

std::string Semantic::tokenTypeToString(const TokenType& _type)const
{
    return m_tokenTypeToString.at(_type);
}

void Semantic::insert_RegexToTokenType(
    std::regex _regex,
    TokenType _tokenType)
{    
    m_tokenTypeToRegex.insert({_tokenType, _regex});
}

void Semantic::insert_StringToTokenType(
    std::string _string,
    TokenType _tokenType)
{
    m_tokenTypeToString[_tokenType] = _string;

    // Clean string before to create a regex
    // TODO: improve it to solve all regex escape char problems
    if (_string.size() == 1) {
        _string.insert(_string.begin(), '[');
        _string.insert(_string.end()  , ']');
    }

    insert_RegexToTokenType(std::regex("^" + _string), _tokenType);
}

void Semantic::insert_TypeToTokenType(Type _type, TokenType _tokenType)
{
    m_tokenTypeToTypeMap.insert({_tokenType, _type});
    m_typeToTokenTypeMap.insert({_type, _tokenType});
}

TokenType Semantic::typeToTokenType(Type _type)const
{
    auto found = m_typeToTokenTypeMap.find(_type);
    if ( found != m_typeToTokenTypeMap.end() )
    {
        return found->second;
    }

    throw std::runtime_error("Semantic not found for this Nodable::Type. Did you insert it using insert_TypeToTokenType() before ?");
}

Type Semantic::tokenTypeToType(TokenType _tokenType)const
{
    auto found = m_tokenTypeToTypeMap.find(_tokenType);
    if ( found != m_tokenTypeToTypeMap.end() )
    {
        return found->second;
    }

    throw std::runtime_error("Semantic not found for this Nodable::TokenType to Nodable::Type. Did you insert it using insert_TypeToTokenType() before ?");
}
