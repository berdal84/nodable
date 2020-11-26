#include "Language.h"
#include "Language/Nodable/NodableLanguage.h"
#include <vector>
#include <Node/Node.h>

using namespace Nodable;

const Language* Language::NODABLE = nullptr;

const Language* Language::Nodable() {

	if (Language::NODABLE == nullptr)
		Language::NODABLE = new NodableLanguage();

	return Language::NODABLE;
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
