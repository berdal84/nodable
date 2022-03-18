#include <nodable/core/Language.h>
#include <vector>
#include <nodable/core/Node.h>
#include <nodable/core/Parser.h>

using namespace Nodable;

Language::~Language()
{
    delete parser;
    delete serializer;
}

void Language::addOperator( InvokableOperator* _operator)
{
	operators.push_back(_operator);
}

bool  Language::hasHigherPrecedenceThan(const InvokableOperator* _firstOperator, const InvokableOperator* _secondOperator)const {
	return _firstOperator->get_precedence() >= _secondOperator->get_precedence();
}

const IInvokable* Language::findFunction(const FunctionSignature* _signature) const
{
	auto predicate = [&](IInvokable* fct) {
		return fct->get_signature()->match(_signature);
	};

	auto it = std::find_if(api.begin(), api.end(), predicate);

	if (it != api.end())
		return *it;

	return nullptr;
}

const InvokableOperator* Language::findOperator(const std::string& _short_identifier) const {

	auto predicate = [&](InvokableOperator* op) {
		return op->get_short_identifier() == _short_identifier;
	};

	auto it = std::find_if(operators.cbegin(), operators.cend(), predicate);

	if (it != operators.end())
		return *it;

	return nullptr;
}

const InvokableOperator* Language::findOperator(const FunctionSignature* _signature) const
{
	
	auto predicate = [&](InvokableOperator* op)
    {
		return _signature->match( op->get_signature() );
	};

	auto found = std::find_if(operators.cbegin(), operators.cend(), predicate );

	if (found != operators.end() )
		return *found;

	return nullptr;
}


void Language::addToAPI(IInvokable* _function)
{
	api.push_back(_function);
	std::string signature;
    serializer->serialize( signature, _function->get_signature() );
	LOG_VERBOSE("Language", "add to API: %s\n", signature.c_str() );
}

const FunctionSignature* Language::createBinOperatorSignature(
    std::shared_ptr<const R::MetaType> _type,
    std::string _identifier,
    std::shared_ptr<const R::MetaType> _ltype,
    std::shared_ptr<const R::MetaType> _rtype) const
{
    auto signature = new FunctionSignature("operator" + _identifier);
    signature->set_return_type(_type);
    signature->push_args(_ltype, _rtype);

    return signature;
}

const FunctionSignature* Language::createUnaryOperatorSignature(
        std::shared_ptr<const R::MetaType> _type,
        std::string _identifier,
        std::shared_ptr<const R::MetaType> _ltype) const
{
    auto signature = new FunctionSignature("operator" + _identifier);
    signature->set_return_type(_type);
    signature->push_args(_ltype);

    return signature;
}
