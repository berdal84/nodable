#include "Language.h"
#include "LanguageNodable.h"
#include "Member.h"
#include <type_traits>
#include <time.h>
#include <vector>

using namespace Nodable;

std::shared_ptr<const Language> Language::Nodable() {

	if (Language::NODABLE == nullptr)
		Language::NODABLE = std::make_shared<const LanguageNodable>();

	return Language::NODABLE;
}

void Language::addOperator( std::shared_ptr<Operator> _operator)
{
	operators.push_back(_operator);
}

void Language::addOperator( std::string       _identifier,
                            unsigned short    _precedence,
                            FunctionSignature _prototype,
                            FunctionImplem  _implementation) {
	auto op = std::make_shared<Operator>(_identifier, _precedence, _prototype, _implementation);
	addOperator( std::move(op) );
}

bool  Language::hasHigherPrecedenceThan(
        const std::shared_ptr<const Operator>& _firstOperator,
        std::shared_ptr<const Operator>        _secondOperator)const
{
	return _firstOperator->precedence >= _secondOperator->precedence;
}

std::shared_ptr<const Function> Nodable::Language::findFunction(const FunctionSignature& _signature) const
{
	auto predicate = [&](const Function& fct) {
		return fct.signature.match(_signature);
	};

	auto it = std::find_if(api.begin(), api.end(), predicate);

	if (it != api.end())
		return *it;

	return nullptr;
}

std::shared_ptr<const Operator> Language::findOperator(const std::string& _identifier) const {

	auto predicate = [&](const Operator& op) {
		return op.identifier == _identifier;
	};

	auto it = std::find_if(operators.cbegin(), operators.cend(), predicate);

	if (it != operators.end())
		return *it;

	return nullptr;
}

std::shared_ptr<const Operator> Language::findOperator(const FunctionSignature& _signature) const {
	
	auto predicate = [&](const Operator& op) {
		return op.signature.match(_signature);
	};

	auto it = std::find_if(operators.cbegin(), operators.cend(), predicate );

	if ( it != operators.end() )
		return *it;

	return nullptr;
}


void Nodable::Language::addToAPI(const std::shared_ptr<Function>& _function)
{
	this->api.push_back(_function);
}

void Nodable::Language::addToAPI(FunctionSignature& _signature, const FunctionImplem& _implementation)
{
	auto f = std::make_shared<Function>( _signature, _implementation );
	this->api.push_back(f);
}
