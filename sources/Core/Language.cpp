#include "Language.h"

using namespace Nodable;

FunctionPrototype::FunctionPrototype(std::string _identifier):identifier(_identifier)
{
}

void FunctionPrototype::pushArgument(TokenType_ _type) {
	arguments.push_back(_type);
}

bool FunctionPrototype::match(FunctionPrototype& _other) {	
	return this->identifier == _other.identifier &&
		   this->arguments == _other.arguments;
}

const std::string& Nodable::FunctionPrototype::getIdentifier()const
{
	return this->identifier;
}

const Language* Language::NODABLE = Language::Nodable();

const Language* Language::Nodable() {

	auto language = new Language();

	language->letters   = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_";

	language->numbers   = "0123456789.";

	language->addOperator( Operator("=", 1u) );
	language->addOperator( Operator("!", 5u) );
	language->addOperator( Operator("-", 10u) );
	language->addOperator( Operator("+", 10u) );
	language->addOperator( Operator("/", 20u) );
	language->addOperator( Operator("*", 20u) );

	language->keywords["true"]  = TokenType_Boolean;
	language->keywords["false"] = TokenType_Boolean;

	return language;
}

void Language::addOperator( Operator _operator) {

	auto item = std::pair<std::string, Operator>(_operator.identifier, _operator);
	operators.insert(item);
}

unsigned short Language::getOperatorPrecedence(const std::string& _identifier)const {

	return operators.at(_identifier).precedence;
}

std::string Nodable::Language::getOperatorsAsString() const
{
	std::string result;

	for (auto it = operators.begin(); it != operators.end(); it++) {
		result.append((*it).second.identifier);
	}

	return result;
}
