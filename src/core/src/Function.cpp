#include <nodable/Function.h>
#include <nodable/Member.h>       // to check member arguments
#include <nodable/Properties.h>   // to use getClass()
#include <nodable/VariableNode.h> // to use mirror::GetClass<Variable>()
#include <nodable/Log.h>

using namespace Nodable;

FunctionArg::FunctionArg(Type _type, std::string _name) {
	type = _type;
	name = _name;
}

FunctionSignature::FunctionSignature(std::string _identifier, Type _type, std::string _label) :
	identifier(_identifier),
	type(_type),
	label(_label)
{

}

void FunctionSignature::pushArg(Type _type, std::string _name) {
	if (_name == "" )
    {
		_name = "arg_" + std::to_string(args.size());
    }
	args.push_back(FunctionArg(_type, _name));
}

bool FunctionSignature::match(const FunctionSignature* _other)const {

    if ( this == _other )
        return true;

	if ( args.size() != _other->args.size() )
		return false;

	if ( identifier != _other->identifier )
	    return false;

	size_t i = 0;
	bool isMatching = true;
	while( i < args.size() && isMatching )
	{
	    if (args[i].type != _other->args[i].type && _other->args[i].type != Type_Any)
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

Type FunctionSignature::getType() const
{
	return type;
}

std::string FunctionSignature::getLabel() const
{
	return label;
}

bool FunctionSignature::hasAtLeastOneArgOfType(Type _type) const
{
    auto found = std::find_if( args.begin(), args.end(), [&_type](const FunctionArg& each) { return  each.type == _type; } );
    return found != args.end();
}
