#include <nodable/core/Language.h>
#include <vector>
#include <nodable/core/Node.h>
#include <nodable/core/Parser.h>
#include <nodable/core/Operator.h>

using namespace Nodable;

Language::~Language()
{
    delete m_parser;
    delete m_serializer;

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
    m_serializer->serialize(signature, _invokable->get_signature() );

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

const Signature* Language::new_binary_op_signature(
    Meta_t _type,
    const Operator* _op,
    Meta_t _ltype,
    Meta_t _rtype
    ) const
{
    std::string id = m_semantic.token_type_to_string(Token_t::keyword_operator) + _op->identifier;
    auto signature = new Signature(id, _op);
    signature->set_return_type(_type);
    signature->push_args(_ltype, _rtype);

    return signature;
}

const Signature* Language::new_unary_op_signature(
        Meta_t _type,
        const Operator* _op,
        Meta_t _ltype) const
{
    std::string id = m_semantic.token_type_to_string(Token_t::keyword_operator) + _op->identifier;
    auto signature = new Signature(id, _op);
    signature->set_return_type(_type);
    signature->push_arg(_ltype);

    return signature;
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
