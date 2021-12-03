#include <nodable/Semantic.h>
#include <nodable/Log.h>

using namespace Nodable;

Semantic::Semantic()
{
    m_type_to_token_type.resize(Type_COUNT);
    m_type_to_string.resize(Type_COUNT);
    m_token_type_to_type.resize(TokenType_COUNT);
    m_token_type_to_string.resize(TokenType_COUNT);

    std::fill(m_token_type_to_type.begin(), m_token_type_to_type.end(), Type_Unknown);
    std::fill(m_type_to_token_type.begin(), m_type_to_token_type.end(), TokenType_Unknown);
    std::fill(m_type_to_string.begin(), m_type_to_string.end(), NULL);
}

void Semantic::insert(const std::regex& _regex, TokenType _tokenType)
{
    m_token_type_regex.push_back(_regex);
    m_regex_index_to_token_type.push_back(_tokenType);
}

void Semantic::insert(const std::regex& _regex, TokenType _tokenType, Type _type)
{
    m_token_type_regex.push_back(_regex);
    m_regex_index_to_token_type.push_back(_tokenType);

    m_type_regex.push_back(_regex);
    m_regex_index_to_type.push_back(_type);

    m_token_type_to_type[_tokenType] = _type;
    m_type_to_token_type[_type]      = _tokenType;
}

void Semantic::insert(std::string _string, Type _type)
{
    m_type_to_string[_type] = _string;
}

void Semantic::insert(std::string _string, TokenType _tokenType, Type _type)
{
    m_token_type_to_type[_tokenType] = _type;
    m_type_to_token_type[_type]      = _tokenType;
    insert(_string, _tokenType);
    insert(_string, _type);
}

void Semantic::insert(std::string _string, TokenType _tokenType)
{
    m_token_type_to_string[_tokenType] = _string;

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
