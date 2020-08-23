#include "Function.h"

using namespace Nodable;

FunctionArg::FunctionArg(TokenType_ _type, std::string _name) {
	type = _type;
	name = _name;
}

FunctionSignature::FunctionSignature(std::string _identifier, TokenType_ _type, std::string _label) :
	identifier(_identifier),
	type(_type),
	label(_label)
{

}

void FunctionSignature::pushArg(TokenType_ _type, std::string _name) {
	if (_name == "")
		_name = "arg_" + std::to_string(args.size());
	args.push_back(FunctionArg(_type, _name));
}

bool FunctionSignature::match(FunctionSignature& _other) {

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

const std::string& FunctionSignature::getIdentifier()const
{
	return this->identifier;
}

FunctionSignature::operator std::string() const
{
	std::string result = identifier + "(";

	for (auto it = args.begin(); it != args.end(); it++) {

		if (it != args.begin())
			result.append(", ");

		if ((*it).type == TokenType_Number)
			result.append("num");
		else if ((*it).type == TokenType_String)
			result.append("str");
		else if ((*it).type == TokenType_Boolean)
			result.append("bool");
		else
			result.append("?");

	}

	result.append(")");

	return result;
}

const std::vector<FunctionArg> FunctionSignature::getArgs() const
{
	return this->args;
}

const TokenType_ FunctionSignature::getType() const
{
	return type;
}

const std::string FunctionSignature::getLabel() const
{
	return label;
}