#include <VariableNode.h>
#include <Member.h>
#include <Log.h>

using namespace Nodable;

VariableNode::VariableNode(): Node("Variable")
{
	props.add("value", Visibility::Always, Type_Any, Way_InOut);
}

VariableNode::~VariableNode()
{

}


void VariableNode::setName(const char* _name)
{
	name = _name;
	setLabel(_name);

	if ( name.length() > 4)
    {
        setShortLabel(name.substr(0,3).append("..").c_str());
    }
    else
    {
        setShortLabel(_name);
    }

    value()->setSourceExpression(_name);
}

const char* VariableNode::getName()const
{
	return name.c_str();
}

bool VariableNode::isType(Type _type)const
{
	return value()->isType(_type);
}

std::string VariableNode::getTypeAsString()const
{
	return value()->getTypeAsString();
}