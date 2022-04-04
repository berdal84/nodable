#include <nodable/core/Language.h>
#include <vector>
#include <nodable/core/Node.h>
#include <nodable/core/Parser.h>

using namespace Nodable;

Language::~Language()
{
    delete m_parser;
    delete m_serializer;

    for( auto each : m_operators ) delete each;
    for( auto each : m_functions ) delete each;
    // for( auto each : m_operator_implems ) delete each; (duplicates from m_functions)
}

bool  Language::has_higher_precedence_than( std::pair<const InvokableOperator*, const InvokableOperator*> _operators)const
{
	return _operators.first->get_precedence() >= _operators.second->get_precedence();
}

const IInvokable* Language::find_function(const FunctionSignature* _signature) const
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

const InvokableOperator* Language::find_operator_fct_exact(const FunctionSignature* _signature) const
{

    auto is_exactly = [&](const IInvokable* _invokable)
    {
        return _signature->is_exactly(_invokable->get_signature());
    };

    auto found = std::find_if(m_operator_implems.cbegin(), m_operator_implems.cend(), is_exactly );

    if (found != m_operator_implems.end() )
    {
        return static_cast<const InvokableOperator*>(*found);
    }

    return nullptr;
}

const InvokableOperator* Language::find_operator_fct(const FunctionSignature* _signature) const
{
	
	auto is_compatible = [&](const IInvokable* _invokable)
    {
		return _signature->is_compatible(_invokable->get_signature());
	};

	auto found = std::find_if(m_operator_implems.cbegin(), m_operator_implems.cend(), is_compatible );

	if (found != m_operator_implems.end() )
    {
        return static_cast<const InvokableOperator*>(*found);
    }

	return nullptr;
}

void Language::add(const IInvokable* _function)
{
	m_functions.push_back(_function);

	std::string signature;
    m_serializer->serialize(signature, _function->get_signature() );

	LOG_VERBOSE("Language", "%s add to functions\n", signature.c_str() );
}

void Language::add(const InvokableOperator* _operator)
{
    NODABLE_ASSERT( std::find( m_operators.begin(), m_operators.end(), _operator->get_operator()) != m_operators.end() )

    m_functions.push_back(_operator);
    m_operator_implems.push_back(_operator);

    std::string signature;
    m_serializer->serialize(signature, _operator->get_signature() );

    LOG_VERBOSE("Language", "%s added to functions and operator implementations\n", signature.c_str() );
}
void Language::add(const Operator* _operator)
{
    NODABLE_ASSERT( std::find( m_operators.begin(), m_operators.end(), _operator) == m_operators.end() )
    m_operators.push_back(_operator);

    std::string str;
    m_serializer->serialize(str, _operator );
    LOG_VERBOSE("Language", "%s added to operators\n", str.c_str() );
}

const FunctionSignature* Language::new_bin_operator_signature(
    Meta_t _type,
    std::string _identifier,
    Meta_t _ltype,
    Meta_t _rtype) const
{
    auto signature = new FunctionSignature("operator" + _identifier);
    signature->set_return_type(_type);
    signature->push_args(_ltype, _rtype);

    return signature;
}

const FunctionSignature* Language::new_unary_operator_signature(
        Meta_t _type,
        std::string _identifier,
        Meta_t _ltype) const
{
    auto signature = new FunctionSignature("operator" + _identifier);
    signature->set_return_type(_type);
    signature->push_args(_ltype);

    return signature;
}

const Operator *Language::find_operator(const std::string& _identifier, Operator_t _type) const
{
    auto is_bin_op_with_expected_identifier = [&](const Operator* op)
    {
        return op->identifier == _identifier && op->type == _type;
    };

    auto found = std::find_if(m_operators.cbegin(), m_operators.cend(), is_bin_op_with_expected_identifier );

    if (found != m_operators.end() )
        return *found;

    return nullptr;
}

const Operator *Language::find_operator(const std::string& _identifier, const FunctionSignature* _signature) const
{
    switch ( _signature->get_arg_count() )
    {
        case 1:  return find_operator(_identifier, Operator_t::Unary);
        case 2:  return find_operator(_identifier, Operator_t::Binary);
        case 3:  return find_operator(_identifier, Operator_t::Ternary);
        default: return nullptr;
    }
}
