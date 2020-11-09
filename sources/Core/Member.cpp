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

void Member::setAllowedConnections(Way _flags)
{
    LOG_MESSAGE(3u, "enableConnector( %i )\n", (int)_flags);

	// Create an input if needed
	if ( _flags == Way::In || _flags == Way::InOut )
    {
		auto conn = std::make_unique<Connector>( std::shared_ptr<Member>(this), Way::In);
		in = std::move(conn);
        LOG_MESSAGE(3u, "allows Way::In\n");
    }
	else
    {
        in.reset();
    }

	// Create an output if needed
	if ( _flags == Way::Out || _flags == Way::InOut )
    {
        auto conn = std::make_unique<Connector>( std::shared_ptr<Member>(this), Way::Out);
        out = std::move(conn);
        LOG_MESSAGE(3u, "allows Way::Out\n");
    }
	else
    {
        out.reset();
    }
}

void Nodable::Member::setSourceExpression(const char* _val)
{
	sourceExpression = _val;
}

void Member::setType(Type _type)
{
    assert(data.isType(Type::Any)); // You cannot change the type twice
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

bool Member::allowsConnections(Way _requiredWay)const
{
    auto way = getConnectorWay();
    auto result = false;

    switch (_requiredWay) {

        case Way::None:
        case Way::InOut:
            result = way == _requiredWay;
            break;
        case Way::In:
        case Way::Out:
            result = way == _requiredWay || way == Way::InOut;
    }

	return  result;
}

Object* Member::getOwner() const
{
	return owner;
}

std::shared_ptr<Member> Member::getInputMember() const
{
	return inputMember;
}

const std::string& Nodable::Member::getName() const
{
	return name;
}

void Member::setInputMember(std::shared_ptr<Member> _val)
{
	inputMember = std::move(_val);

	if (_val == nullptr)
		sourceExpression = "";
}

void Nodable::Member::setName(const char* _name)
{
	name = _name;
}

Way Member::getConnectorWay() const
{
    LOG_MESSAGE(3u, "getConnectorWay() - Set Way::None by default\n");
    Way way = Way::None;

	if ( in != nullptr )
    {
	    way = Way::In;
        LOG_MESSAGE(3u, "getConnectorWay() - Set Way::In because this->in is not nullptr\n");
    }

	if ( out != nullptr )
	{
        if ( way == Way::In )
        {
            way = Way::InOut;
            LOG_MESSAGE(3u, "getConnectorWay() - Set Way::InOut because this->out is not nullptr too\n");

        }
        else
        {
            way = Way::Out;
            LOG_MESSAGE(3u, "getConnectorWay() - Set Way::Out because this->out is not nullptr\n");

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

	if (allowsConnections(Way::In) && inputMember != nullptr)
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

void Member::set(const std::shared_ptr<Member>& _v)
{
	data = _v->data;
}

void Member::set(double _value)
{
	data.set(_value);
}

void Member::set(int _value)
{
	set(double(_value));
}

void Member::set(std::string _value)
{
	data.set(_value.c_str());
}

void Member::set(const char* _value)
{
	data.set(_value);
}

void Member::set(bool _value)
{
	data.set(_value);
}
