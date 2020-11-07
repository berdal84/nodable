#include "Member.h"
#include "Log.h"		 // for LOG_DEBUG(...)
#include "Object.h"
#include "Variable.h"
#include "Language.h"

using namespace Nodable;

Type Member::getType()const
{
	return data.getType();
}

bool Member::isEditable() const
{
    return this->getInputMember() == nullptr;
}

bool  Member::isType(Type _type)const
{
	return data.isType(_type);
}

bool Member::equals(const Member *_other)const {
	return _other != nullptr &&
	       _other->isType(this->getType() ) &&
		   (std::string)*_other == (std::string)*this;
}

void Member::setConnectorWay(Way _flags)
{
    in.reset();
    out.reset();

	// Create an input if needed
	if (_flags & Way_In)
    {
		auto conn = std::make_unique<Connector>(this, Way_In);
		in = std::move(conn);
    }

	// Create an output if needed
	if (_flags & Way_Out)
    {
        auto conn = std::make_unique<Connector>(this, Way_Out);
        in = std::move(conn);
    }
}

void Nodable::Member::setSourceExpression(const char* _val)
{
	sourceExpression = _val;
}

void Member::setType(Type _type)
{
	data.setType(_type);
}

void Member::setVisibility(Visibility _v)
{
	visibility = _v;
}

void Nodable::Member::updateValueFromInputMemberValue()
{
	this->set(this->inputMember);
}

bool Member::allows(Way _way)const
{
	auto maskedFlags = getConnectorWay() & _way;
	return ((Way)maskedFlags) == _way;
}

Object* Member::getOwner() const
{
	return owner;
}

Member* Member::getInputMember() const
{
	return inputMember;
}

const std::string& Nodable::Member::getName() const
{
	return name;
}

void Member::setInputMember(Member* _val)
{
	inputMember = _val;

	if (_val == nullptr)
		sourceExpression = "";
}

void Nodable::Member::setName(const char* _name)
{
	name = _name;
}

Way Member::getConnectorWay() const
{
    Way way = Way_None;

	if ( in != nullptr )
    {
	    way = Way_In;
    }

	if ( out != nullptr )
	{
        if ( way == Way_In )
        {
            way = Way_InOut;
        }
        else
        {
            way = Way_Out;
        }
	}

	return way;
}

bool Member::isSet()const
{
	return data.isSet();
}

void Nodable::Member::setOwner(Object* _owner)
{
	owner = _owner;
}

std::string Member::getTypeAsString()const
{
	return data.getTypeAsString();
}

std::string Member::getSourceExpression()const
{
	std::string expression;

	if ( allows(Way_In) && inputMember != nullptr)
	{
		// if inputMember is a variable we add the variable name and an equal sign
		if (inputMember->getOwner()->getClass()->getName() == "Variable" &&
			getOwner()->getClass()->getName() == "Variable")
		{
			auto variable = inputMember->getOwner()->as<Variable>();
			expression.append(variable->getName());
			expression.append("=");
			expression.append(inputMember->getSourceExpression());

		}else
			expression = inputMember->getSourceExpression();

	} else if (sourceExpression != "") {
		expression = sourceExpression;

	} else {

		if (isType(Type::String)) {
			expression = '"' + (std::string)*this + '"';
		}
		else {
			expression = (std::string)*this;
		}
	}

	return expression;
}


void Member::set(const Member* _v)
{
	data.set(&_v->data);
}

void Member::set(const Member& _v)
{
	data.set(&_v.data);
}

void Member::set(double _value)
{
	data.setType(Type::Double);
	data.set(_value);
}

void Member::set(int _value)
{
	set(double(_value));
}

void Member::set(const std::string& _value)
{
	this->set(_value.c_str());
}

void Member::set(const char* _value)
{
	data.setType(Type::String);
	data.set(_value);
}

void Member::set(bool _value)
{
	data.setType(Type::Boolean);
	data.set(_value);
}
