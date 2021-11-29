#include <nodable/Language.h>
#include <vector>
#include <nodable/Node.h>
#include <nodable/Parser.h>

using namespace Nodable;

Language::~Language()
{
    delete parser;
    delete serializer;
}

void Language::addOperator( Operator* _operator)
{
	operators.push_back(_operator);
}

bool  Language::hasHigherPrecedenceThan(const Operator* _firstOperator, const Operator* _secondOperator)const {
	return _firstOperator->getPrecedence() >= _secondOperator->getPrecedence();
}

const Invokable* Language::findFunction(const FunctionSignature* _signature) const
{
	auto predicate = [&](Invokable* fct) {
		return fct->getSignature()->match(_signature);
	};

	auto it = std::find_if(api.begin(), api.end(), predicate);

	if (it != api.end())
		return *it;

	return nullptr;
}

const Operator* Language::findOperator(const std::string& _short_identifier) const {

	auto predicate = [&](Operator* op) {
		return op->getShortIdentifier() == _short_identifier;
	};

	auto it = std::find_if(operators.cbegin(), operators.cend(), predicate);

	if (it != operators.end())
		return *it;

	return nullptr;
}

const Operator* Language::findOperator(const FunctionSignature* _signature) const {
	
	auto predicate = [&](Operator* op) {
		return op->getSignature()->match(_signature);
	};

	auto it = std::find_if(operators.cbegin(), operators.cend(), predicate );

	if ( it != operators.end() )
		return *it;

	return nullptr;
}


void Language::addToAPI(Invokable* _function)
{
	api.push_back(_function);
	std::string signature;
    serializer->serialize( signature, _function->getSignature() );
	LOG_VERBOSE("Language", "add to API: %s\n", signature.c_str() );
}

const FunctionSignature* Language::createBinOperatorSignature(
        Type _type,
        std::string _identifier,
        Type _ltype,
        Type _rtype) const
{
    auto tokType  = semantic.typeToTokenType(_type);
    auto tokLType = semantic.typeToTokenType(_ltype);
    auto tokRType = semantic.typeToTokenType(_rtype);

    auto signature = new FunctionSignature("operator" + _identifier, tokType);

    signature->pushArgs(tokLType, tokRType);

    return signature;
}

const FunctionSignature* Language::createUnaryOperatorSignature(
        Type _type,
        std::string _identifier,
        Type _ltype) const
{
    auto tokType = semantic.typeToTokenType(_type);
    auto tokLType = semantic.typeToTokenType(_ltype);

    auto signature = new FunctionSignature("operator" + _identifier, tokType);

    signature->pushArgs(tokLType);

    return signature;
}
