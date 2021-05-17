#include <nodable/Function.h>
#include <nodable/Member.h>       // to check member arguments
#include <nodable/Properties.h>   // to use getClass()
#include <nodable/VariableNode.h> // to use mirror::GetClass<Variable>()
#include <nodable/Log.h>

using namespace Nodable::core;

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
	while(i < args.size() && isMatching)
	{
			if (args[i].type != _other.args[i].type && _other.args[i].type != TokenType_AnyType)
				isMatching = false;
		i++;
	}

	return isMatching;
}

const std::string& FunctionSignature::getIdentifier()const
{
	return this->identifier;
}

std::vector<FunctionArg> FunctionSignature::getArgs() const
{
	return this->args;
}

TokenType FunctionSignature::getType() const
{
	return type;
}

std::string FunctionSignature::getLabel() const
{
	return label;
}

bool FunctionSignature::hasAtLeastOneArgOfType(TokenType _type)
{
    auto found = std::find_if( args.begin(), args.end(), [&_type](FunctionArg& each) { return  each.type == _type; } );
    return found != args.end();
}
