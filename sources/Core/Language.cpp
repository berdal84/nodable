#include "Language.h"
#include "LanguageNodable.h"
#include "Member.h"
#include <type_traits>
#include <time.h>
#include <vector>

using namespace Nodable;

const Language* Language::NODABLE = nullptr;

const Language* Language::Nodable() {

	if (Language::NODABLE == nullptr)
		Language::NODABLE = new LanguageNodable();

	return Language::NODABLE;
}

void Language::addOperator( Operator _operator) {

	auto item = std::pair<std::string, Operator>(_operator.identifier, _operator);
	operators.insert(item);
	//keywordToTokenType[_operator.identifier] = TokenType::Operator;
}

void Language::addOperator( std::string       _identifier,
                            unsigned short    _precedence,
                            FunctionSignature _prototype,
                            FunctionImplem  _implementation) {
	Operator op(_identifier, _precedence, _prototype, _implementation);
	addOperator(op);
}

unsigned short Language::getOperatorPrecedence(const std::string& _identifier)const {

	return operators.at(_identifier).precedence;
}

bool  Language::needsToBeEvaluatedFirst(std::string op, std::string nextOp)const {
	return getOperatorPrecedence(op) > getOperatorPrecedence(nextOp);
}

std::string Nodable::Language::getOperatorsAsString() const
{
	std::string result;

	for (auto it = operators.begin(); it != operators.end(); it++) {
		result.append((*it).second.identifier);
	}

	return result;
}

const Function* Nodable::Language::findFunction(FunctionSignature& _prototype) const
{
	auto predicate = [&](Function fct) {
		return fct.signature.match(_prototype);
	};

	auto it = std::find_if(api.begin(), api.end(), predicate);

	if (it != api.end())
		return &*it;

	return nullptr;
}

const Operator* Language::findOperator(const std::string& _operator) const {
	
	auto it = operators.find(_operator);
	if ( it != operators.end() )
		return &it->second;

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
