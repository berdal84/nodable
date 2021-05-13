#include "Language.h"
#include <vector>
#include <Node/Node.h>
#include <Language/Common/Parser.h>

using namespace Nodable;

Language::~Language()
{
    delete parser;
    delete serializer;
}

void Language::addOperator( Operator _operator)
{
	operators.push_back(_operator);
}

void Language::addOperator( std::string       _identifier,
                            unsigned short    _precedence,
                            FunctionSignature _prototype,
                            FunctionImplem  _implementation) {
	Operator op(_identifier, _precedence, _prototype, _implementation);
	addOperator(op);
}

bool  Language::hasHigherPrecedenceThan(const Operator* _firstOperator, const Operator* _secondOperator)const {
	return _firstOperator->precedence >= _secondOperator->precedence;
}

const Function* Nodable::Language::findFunction(const FunctionSignature& _signature) const
{
	auto predicate = [&](Function fct) {
		return fct.signature.match(_signature);
	};

	auto it = std::find_if(api.begin(), api.end(), predicate);

	if (it != api.end())
		return &*it;

	return nullptr;
}

const Operator* Language::findOperator(const std::string& _identifier) const {

	auto predicate = [&](Operator op) {
		return op.identifier == _identifier;
	};

	auto it = std::find_if(operators.cbegin(), operators.cend(), predicate);

	if (it != operators.end())
		return &*it;

	return nullptr;
}

const Operator* Language::findOperator(const FunctionSignature& _signature) const {
	
	auto predicate = [&](Operator op) {
		return op.signature.match(_signature);
	};

	auto it = std::find_if(operators.cbegin(), operators.cend(), predicate );

	if ( it != operators.end() )
		return &*it;

	return nullptr;
}


void Nodable::Language::addToAPI(Function _function)
{
	this->api.push_back(_function);
}

void Nodable::Language::addToAPI(FunctionSignature& _signature, FunctionImplem _implementation)
{
	Function f(_signature, _implementation);
	this->api.push_back(f);
}

const FunctionSignature Language::createBinOperatorSignature(
        Type _type,
        std::string _identifier,
        Type _ltype,
        Type _rtype) const
{
    auto tokType  = semantic.typeToTokenType(_type);
    auto tokLType = semantic.typeToTokenType(_ltype);
    auto tokRType = semantic.typeToTokenType(_rtype);

    FunctionSignature signature("operator" + _identifier, tokType);

    signature.pushArgs(tokLType, tokRType);

    return signature;
}

const FunctionSignature Language::createUnaryOperatorSignature(
        Type _type,
        std::string _identifier,
        Type _ltype) const
{
    auto tokType = semantic.typeToTokenType(_type);
    auto tokLType = semantic.typeToTokenType(_ltype);

    FunctionSignature signature("operator" + _identifier, tokType);

    signature.pushArgs(tokLType);

    return signature;
}
