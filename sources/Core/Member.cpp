#include "Member.h"

#include <utility>
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
    // This member will be editble by the user only if it has no inputConnectedMember
    return this->inputMember.expired() || this->inputMember.lock() == nullptr;
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
		auto conn = std::make_unique<Connector>( this->weak_from_this() , Way::In);
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
        auto conn = std::make_unique<Connector>( this->weak_from_this(), Way::Out);
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
    if ( this->inputMember.expired() )
    {
        LOG_WARNING(0u, "Unable to update %s member value because its input connected node has expired.", getName().c_str() );
        this->inputMember.reset();
    }
    else
    {
        this->set(this->inputMember.lock());
    }
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

std::shared_ptr<Object> Member::getOwner() const
{
	return owner.lock();
}

std::shared_ptr<Member> Member::getInputConnectedMember() const
{
	return inputMember.lock();
}

const std::string& Nodable::Member::getName() const
{
	return name;
}

void Member::resetInputConnectedMember()
{
    inputMember.reset();
    sourceExpression = "";
}

void Member::setInputConnectedMember(std::weak_ptr<Member> _val)
{
	inputMember = std::move(_val);
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

void Nodable::Member::setOwner( std::weak_ptr<Object> _owner)
{
	owner = std::move(_owner);
}

std::string Member::getTypeAsString()const
{
	return data.getTypeAsString();
}

std::string Member::getSourceExpression()const
{
	std::string expression;

	if (allowsConnections(Way::In) && !inputMember.expired() )
	{
	    auto _inputMember = inputMember.lock();
	    auto _inputNode = _inputMember->getOwner();

		// if inputMember is a variable we add the variable name and an equal sign
		if (_inputNode->getClass()->getName() == "Variable" &&
			getOwner()->getClass()->getName() == "Variable")
		{
			auto variable = std::static_pointer_cast<Variable>(_inputNode);
			expression.append(variable->getName());
			expression.append("=");
			expression.append(_inputMember->getSourceExpression());
		}
		else
        {
            expression = _inputMember->getSourceExpression();
        }

	}
	else if (!sourceExpression.empty())
	{
		expression = sourceExpression;
	}
	else
    {

		if (isType(Type::String))
		{
			expression = '"' + (std::string)*this + '"';
		}
		else
        {
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
