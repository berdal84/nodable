#include "Language.h"

using namespace Nodable;

FunctionArg::FunctionArg(TokenType_ _type, std::string _name) {
	type = _type;
	name = _name;
}

FunctionPrototype::FunctionPrototype(std::string _identifier, TokenType_ _type):
	identifier(_identifier),
	type(_type)
{

}

void FunctionPrototype::pushArg(TokenType_ _type, std::string _name) {

	args.push_back( FunctionArg(_type, _name) );	
}

bool FunctionPrototype::match(FunctionPrototype& _other) {	

	if (identifier != _other.identifier)
		return false;

	if (args.size() != _other.args.size())
		return false;

	for (size_t i = 0; i < args.size(); i++) {
		if (args[i].type != _other.args[i].type)
			return false;
	}

	return true;
}

const std::string& FunctionPrototype::getIdentifier()const
{
	return this->identifier;
}

const std::vector<FunctionArg> FunctionPrototype::getArgs() const
{
	return this->args;
}

const TokenType_ FunctionPrototype::getType() const
{
	return type;
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

	/* Function library */

	{
		FunctionPrototype proto("nothing", TokenType_Number);
		proto.pushArg(TokenType_Number, "input");
		language->pushFunc(proto);
	}

	{
		FunctionPrototype proto("sin", TokenType_Number);
		proto.pushArg(TokenType_Number);
		language->pushFunc(proto);
	}

	{
		FunctionPrototype proto("cos", TokenType_Number);
		proto.pushArg(TokenType_Number);
		language->pushFunc(proto);
	}

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

const FunctionPrototype* Nodable::Language::findFunctionPrototype(FunctionPrototype& _prototype) const
{
	auto predicate = [&](FunctionPrototype p) {
		return p.match(_prototype);
	};

	auto it = std::find_if(functionPrototypes.begin(), functionPrototypes.end(), predicate);

	if (it != functionPrototypes.end())
		return &*it;

	return nullptr;
}

void Nodable::Language::pushFunc(FunctionPrototype prototype)
{
	this->functionPrototypes.push_back(prototype);
}
