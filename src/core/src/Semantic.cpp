#include <nodable/core/Semantic.h>
#include <nodable/core/Log.h>

using namespace Nodable;

Semantic::Semantic()
{
}

void Semantic::insert(const std::regex& _regex, Token_t _tokenType)
{
    m_token_type_regex.push_back(_regex);
    m_regex_index_to_token_type.push_back(_tokenType);
}

void Semantic::insert(const std::regex& _regex, Token_t _tokenType, type _type)
{
    m_token_type_regex.push_back(_regex);
    m_regex_index_to_token_type.push_back(_tokenType);

    m_type_regex.push_back(_regex);
    m_regex_index_to_type.push_back(_type);

    m_token_type_to_type.insert({_tokenType, _type});
    m_type_to_token_type.insert({_type.hash_code(), _tokenType});
}

void Semantic::insert(std::string _string, type _type)
{
    m_type_to_string[_type.hash_code()] = _string;
}

void Semantic::insert(std::string _string, Token_t _tokenType, type _type)
{
    m_token_type_to_type.insert({_tokenType, _type});
    m_type_to_token_type.insert({_type.hash_code(), _tokenType});
    insert(_string, _tokenType);
    insert(_string, _type);
}

void Semantic::insert(std::string _string, Token_t _tokenType)
{
    m_token_type_to_string.insert({_tokenType, _string});

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
