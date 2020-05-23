#include "Language.h"
#include "Member.h"
#include <type_traits>
#include <time.h>
#include <vector>

using namespace Nodable;

FunctionArg::FunctionArg(TokenType_ _type, std::string _name) {
	type = _type;
	name = _name;
}

FunctionPrototype::FunctionPrototype(std::string _identifier, TokenType_ _type):
	identifier(_identifier),
	type(_type),
	nativeFunction(NULL)
{

}

void FunctionPrototype::pushArg(TokenType_ _type) {

	std::string argName = "arg_" + std::to_string(args.size());
	args.push_back( FunctionArg(_type, argName) );
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

const Language* Language::NODABLE = nullptr;

const Language* Language::Nodable() {

	if (Language::NODABLE != nullptr)
		return Language::NODABLE;

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
		FunctionPrototype proto("returnNumber", TokenType_Number);
		proto.pushArg(TokenType_Number);

		proto.nativeFunction = [](Member* _result, const std::vector<const Member*>& _args)->int {
			_result->set(_args[0]->as<double>());
			return 0;
		};

		language->pushFunc(proto);
	}

	{
		FunctionPrototype proto("sin", TokenType_Number);
		proto.pushArg(TokenType_Number);

		proto.nativeFunction = [](Member* _result, const std::vector<const Member*>& _args)->int {
			_result->set( sin(_args[0]->as<double>()) );
			return 0;
		};

		language->pushFunc(proto);
	}

	{
		FunctionPrototype proto("cos", TokenType_Number);
		proto.pushArg(TokenType_Number);

		proto.nativeFunction = [](Member* _result, const std::vector<const Member*>& _args)->int {
			_result->set(cos(_args[0]->as<double>()));
			return 0;
		};

		language->pushFunc(proto);
	}

	{
		FunctionPrototype proto("add", TokenType_Number);
		proto.pushArg(TokenType_Number);
		proto.pushArg(TokenType_Number);

		proto.nativeFunction = [](Member* _result, const std::vector<const Member*>& _args)->int {
			_result->set( _args[0]->as<double>() + _args[1]->as<double>());
			return 0;
		};

		language->pushFunc(proto);
	}

	{
		FunctionPrototype proto("minus", TokenType_Number);
		proto.pushArg(TokenType_Number);
		proto.pushArg(TokenType_Number);

		proto.nativeFunction = [](Member* _result, const std::vector<const Member*>& _args)->int {
			_result->set(_args[0]->as<double>() - _args[1]->as<double>());
			return 0;
		};

		language->pushFunc(proto);
	}

	{
		FunctionPrototype proto("mult", TokenType_Number);
		proto.pushArg(TokenType_Number);
		proto.pushArg(TokenType_Number);

		proto.nativeFunction = [](Member* _result, const std::vector<const Member*>& _args)->int {
			_result->set(_args[0]->as<double>() * _args[1]->as<double>());
			return 0;
		};

		language->pushFunc(proto);
	}

	{
		FunctionPrototype proto("sqrt", TokenType_Number);
		proto.pushArg(TokenType_Number);
		proto.nativeFunction = [](Member* _result, const std::vector<const Member*>& _args)->int {
			_result->set( sqrt(_args[0]->as<double>()));
			return 0;
		};
		language->pushFunc(proto);
	}

	{
		FunctionPrototype proto("pow", TokenType_Number);
		proto.pushArg(TokenType_Number);
		proto.pushArg(TokenType_Number);
		proto.nativeFunction = [](Member* _result, const std::vector<const Member*>& _args)->int {
			const auto value = pow(_args[0]->as<double>(), _args[1]->as<double>());
			_result->set(value);
			return 0;
		};

		language->pushFunc(proto);
	}

	{
		FunctionPrototype proto("DNAtoAninoAcid", TokenType_String);
		proto.pushArg(TokenType_String);
		proto.nativeFunction = [](Member* _result, const std::vector<const Member*>& _args)->int {
			
			std::string value = "<TODO>";
			if (_args[0]->as<std::string>() == "UAA" ||
				_args[0]->as<std::string>() == "UAG" ||
				_args[0]->as<std::string>() == "UGA") {

				value = "Stop";
			}

			_result->set(value);
			return 0;
		};
		language->pushFunc(proto);
	}

	{
		FunctionPrototype proto("time", TokenType_Number);
		proto.nativeFunction = [](Member* _result, const std::vector<const Member*>& _args)->int {
			time_t rawtime;
			struct tm* timeinfo;
			time(&rawtime);
			timeinfo = localtime(&rawtime);
			_result->set((double)rawtime);
			return 0;
		};

		language->pushFunc(proto);
	}

	Language::NODABLE = language;

	return language;
}

void Language::addOperator( Operator _operator) {

	auto item = std::pair<std::string, Operator>(_operator.identifier, _operator);
	operators.insert(item);
}

unsigned short Language::getOperatorPrecedence(const std::string& _identifier)const {

	return operators.at(_identifier).precedence;
}

bool  Language::needsToBeEvaluatedFirst(std::string op, std::string nextOp)const {
	return getOperatorPrecedence(op) >= getOperatorPrecedence(nextOp);
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
