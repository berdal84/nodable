#include "Function.h"

using namespace Nodable;

FunctionArg::FunctionArg(TokenType _type, std::string _name) {
	type = _type;
	name = _name;
}

FunctionSignature::FunctionSignature(std::string _identifier, TokenType _type, std::string _label) :
	identifier(_identifier),
	type(_type),
	label(_label)
{

}

void FunctionSignature::pushArg(TokenType _type, std::string _name) {
	if (_name == "")
		_name = "arg_" + std::to_string(args.size());
	args.push_back(FunctionArg(_type, _name));
}

bool FunctionSignature::match(const FunctionSignature& _other)const {

	if (identifier != _other.identifier)
		return false;

	if (args.size() != _other.args.size())
		return false;

	size_t i = 0;
	bool isMatching = true;
	while(i < args.size() && isMatching) {
		if (_other.args[i].type != TokenType::AnyType) // in case argument's type is unknown we allow casting
			if (args[i].type != _other.args[i].type)
				isMatching = false;
		i++;
	}

	return isMatching;
}

const std::string& FunctionSignature::getIdentifier()const
{
	return this->identifier;
}

const std::vector<FunctionArg> FunctionSignature::getArgs() const
{
	return this->args;
}

const TokenType FunctionSignature::getType() const
{
	return type;
}

const std::string FunctionSignature::getLabel() const
{
	return label;
}