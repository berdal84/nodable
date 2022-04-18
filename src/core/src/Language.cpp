#include <nodable/core/Language.h>
#include <vector>
#include <nodable/core/Node.h>
#include <nodable/core/Parser.h>
#include <nodable/core/Operator.h>
#include <nodable/core/IParser.h>
#include <nodable/core/ISerializer.h>

using namespace Nodable;

Language::~Language()
{
    for( auto each : m_operators ) delete each;
    for( auto each : m_functions ) delete each;
    // for( auto each : m_operator_implems ) delete each; (duplicates from m_functions)
}

const IInvokable* Language::find_function(const Signature* _signature) const
{
	auto is_compatible = [&](const IInvokable* fct)
    {
		return fct->get_signature()->is_compatible(_signature);
	};

	auto it = std::find_if(m_functions.begin(), m_functions.end(), is_compatible);

	if (it != m_functions.end())
    {
        return *it;
    }

	return nullptr;
}

const IInvokable* Language::find_operator_fct_exact(const Signature* _signature) const
{
    if(!_signature)
    {
        return nullptr;
    }

    auto is_exactly = [&](const IInvokable* _invokable)
    {
        return _signature->is_exactly(_invokable->get_signature());
    };

    auto found = std::find_if(m_operator_implems.cbegin(), m_operator_implems.cend(), is_exactly );

    if (found != m_operator_implems.end() )
    {
        return *found;
    }

    return nullptr;
}

const IInvokable* Language::find_operator_fct(const Signature* _signature) const
{
    if(!_signature)
    {
        return nullptr;
    }
    auto exact = find_operator_fct_exact(_signature);
    if( !exact) return find_operator_fct_fallback(_signature);
    return exact;
}

const IInvokable* Language::find_operator_fct_fallback(const Signature* _signature) const
{
	
	auto is_compatible = [&](const IInvokable* _invokable)
    {
		return _signature->is_compatible(_invokable->get_signature());
	};

	auto found = std::find_if(m_operator_implems.cbegin(), m_operator_implems.cend(), is_compatible );

	if (found != m_operator_implems.end() )
    {
        return *found;
    }

	return nullptr;
}


void Language::add_invokable(const IInvokable* _invokable)
{
    m_functions.push_back(_invokable);

    std::string signature;
    m_serializer.serialize(signature, _invokable->get_signature() );

    if( _invokable->get_signature()->is_operator() )
    {
        auto found = std::find(m_operator_implems.begin(), m_operator_implems.end(), _invokable);
        NODABLE_ASSERT( found == m_operator_implems.end() )
        m_operator_implems.push_back(_invokable);

        LOG_VERBOSE("Language", "%s added to functions and operator implems\n", signature.c_str() );
    }
    else
    {
        LOG_VERBOSE("Language", "%s added to functions\n", signature.c_str() );
    }
}

void Language::add_operator(const char* _id, Operator_t _type, int _precedence)
{
    const Operator* op = new Operator(_id, _type, _precedence);

    NODABLE_ASSERT( std::find( m_operators.begin(), m_operators.end(), op) == m_operators.end() )
    m_operators.push_back(op);
}

const Operator* Language::find_operator(const std::string& _identifier, Operator_t _type) const
{
    auto is_exactly = [&](const Operator* op)
    {
        return op->identifier == _identifier && op->type == _type;
    };

    auto found = std::find_if(m_operators.cbegin(), m_operators.cend(), is_exactly );

    if (found != m_operators.end() )
        return *found;

    return nullptr;
}


void Language::add_regex(const std::regex& _regex, Token_t _token_t)
{
    m_token_regex.push_back(_regex);
    m_regex_to_token.push_back(_token_t);
}

void Language::add_regex(const std::regex& _regex, Token_t _token_t, type _type)
{
    m_token_regex.push_back(_regex);
    m_regex_to_token.push_back(_token_t);

    m_type_regex.push_back(_regex);
    m_regex_to_type.push_back(_type);

    m_token_to_type.insert({_token_t, _type});
    m_type_to_token.insert({_type.hash_code(), _token_t});
}

void Language::add_type(type _type, std::string _string)
{
    m_type_to_string[_type.hash_code()] = _string;
}

void Language::add_type(type _type, Token_t _token_t, std::string _string)
{
    m_token_to_type.insert({_token_t, _type});
    m_type_to_token.insert({_type.hash_code(), _token_t});
    add_string(_string, _token_t);
    add_type(_type, _string);
}

void Language::add_string(std::string _string, Token_t _token_t)
{
    m_token_to_string.insert({_token_t, _string});
    add_regex(std::regex("^(" + _string + ")"), _token_t);
}

void Language::add_char(const char _char, Token_t _token_t)
{
    m_token_to_char.insert({_token_t, _char});
    m_char_to_token.insert({_char, _token_t});
}


const Signature* Language::new_operator_signature(
        type _type,
        const Operator* _op,
        type _ltype,
        type _rtype
) const
{
    if(!_op)
    {
        return nullptr;
    }

    auto signature = new Signature( sanitize_operator_id(_op->identifier), _op);
    signature->set_return_type(_type);
    signature->push_args(_ltype, _rtype);

    NODABLE_ASSERT(signature->is_operator())
    NODABLE_ASSERT(signature->get_arg_count() == 2)

    return signature;
}

const Signature* Language::new_operator_signature(
        type _type,
        const Operator* _op,
        type _ltype) const
{
    if(!_op)
    {
        return nullptr;
    }

    auto signature = new Signature( sanitize_operator_id(_op->identifier), _op);
    signature->set_return_type(_type);
    signature->push_arg(_ltype);

    NODABLE_ASSERT(signature->is_operator())
    NODABLE_ASSERT(signature->get_arg_count() == 1)

    return signature;
}

std::string& Language::to_string(std::string& _out, type _type) const
{
    auto found = m_type_to_string.find(_type.hash_code());
    if( found != m_type_to_string.cend() )
    {
        return _out.append( found->second );
    }
    return _out;
}

std::string& Language::to_string(std::string& _out, Token_t _token_t) const
{
    {
        auto found = m_token_to_char.find(_token_t);
        if (found != m_token_to_char.cend())
        {
            _out.push_back( found->second );
            return _out;
        }
    }
    auto found = m_token_to_string.find(_token_t);
    if (found != m_token_to_string.cend())
    {
        return _out.append( found->second );
    }
    return _out;
}

std::string Language::to_string(type _type) const
{
    std::string result;
    return to_string(result, _type);
}

std::string Language::to_string(Token_t _token) const
{
    std::string result;
    return to_string(result, _token);
}
