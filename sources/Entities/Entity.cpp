#include "Entity.h"
#include "Log.h"		// for LOG_DBG(...)
#include "NodeView.h"
#include <algorithm>    // for std::find
#include "Wire.h"
#include "WireView.h"

using namespace Nodable;

void Entity::Disconnect(Wire* _wire)
{
	auto sourceNode = _wire->getSource()->getOwner()->getAs<Entity*>();
	auto targetNode = _wire->getTarget()->getOwner()->getAs<Entity*>();


	// remove wire pointer from sourceNode's wires.
	{
		auto found = std::find(sourceNode->wires.begin(), sourceNode->wires.end(), _wire);
		NODABLE_VERIFY(found != sourceNode->wires.end());
		sourceNode->wires.erase(found);
	}

	// remove wire pointer from targetNode's wires.
	{
		auto found = std::find(targetNode->wires.begin(), targetNode->wires.end(), _wire);
		NODABLE_VERIFY(found != targetNode->wires.end());
		targetNode->wires.erase(found);
	}

	_wire->setTarget(nullptr);
	_wire->setSource(nullptr);

	return;

}

void Entity::Connect( Wire* _wire,
					Value* _from,
					Value* _to)
{	
	// Connect wire's source and target to nodes _from and _to.
	_wire->setSource(_from);
	_wire->setTarget(_to);

	_from->getOwner()->getAs<Entity*>()->wires.push_back(_wire);
	_to->getOwner()->getAs<Entity*>()->wires.push_back(_wire);
}

Entity::~Entity()
{
	for(auto wire : wires)
	{
		Entity::Disconnect(wire);
	}
}

void Entity::addComponent(const std::string&  _componentName, Component*  _component)
{
	components[_componentName] = _component;
	_component->setOwner(this);
}

bool Entity::hasComponent(const std::string&  _componentName)const
{
	auto it = components.find(_componentName);
	return it != components.end();
}

Component* Entity::getComponent(const std::string&  _componentName)const
{
	return components.at(_componentName);
}

void Entity::removeComponent(const std::string& _componentName)
{
	components.erase(_componentName);
}

bool Entity::isDirty()const
{
	return dirty;
}

void Entity::setDirty(bool _value)
{
	// Propagate thru output wires only if the node is no already dirty.
	// node: if this node is already dirty, all its output should already be dirty too.
	if (!dirty)
	{
		for(auto wire : wires)
		{
			if (wire->getSource()->getOwner() == this && wire->getTarget() != nullptr)
			{
				auto node = (Entity*)wire->getTarget()->getOwner();
				node->setDirty(_value);
			}
		}
	}

	dirty = _value;
}

Container* Entity::getParent()const
{
	return this->parent;
}

void Entity::setParent(Container* _container)
{
	this->parent = _container;
}

void Entity::setLabel(const char* _label)
{
	this->label = _label;
}

void Entity::setLabel(std::string _label)
{
	this->label = _label;
}

const char* Entity::getLabel()const
{
	return this->label.c_str();
}

std::vector<Wire*>& Entity::getWires()
{
	return wires;
}

int Entity::getInputWireCount()const
{
	int count = 0;
	for(auto w : wires)
	{
		if ( w->getTarget()->getOwner() == this)
			count++;
	}
	return count;
}

int Entity::getOutputWireCount()const
{
	int count = 0;
	for(auto w : wires)
	{
		if ( w->getSource()->getOwner() == this)
			count++;
	}
	return count;
}

bool Entity::update()
{
	bool success = true;

	// Evaluates only if dirty flag is on
	if (isDirty())
	{
		// first we need to evaluate each input and transmit its results thru the wire
		for (auto wire : wires)
		{
			if ( wire->getTarget()->getOwner() == this && wire->getSource() != nullptr)
			{
				auto source = (Entity*)wire->getSource()->getOwner();
				source->update();
				wire->transmitData();
			}
		}

		// then we evaluates this node
		if(hasComponent("operation"))
			getComponent("operation")->update();
		
		if(hasComponent("dataAccess"))
			getComponent("dataAccess")->update();

		setDirty(false);
	}

	return success;
}

void Entity::onMemberValueChanged(const char* _name)
{
	setDirty(true);
	updateLabel();	
}