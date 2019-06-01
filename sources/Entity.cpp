#include "Entity.h"
#include "Log.h"		// for LOG_DBG(...)
#include "NodeView.h"
#include <algorithm>    // for std::find
#include "Wire.h"
#include "WireView.h"
#include "History.h"

using namespace Nodable;

void Entity::Disconnect(Wire* _wire)
{
	_wire->getTarget()->setInput(nullptr);
	_wire->getTarget()->getOwner()->getAs<Entity*>()->setDirty();

	_wire->setTarget(nullptr);
	_wire->setSource(nullptr);

	return;
}

void Entity::Connect( Wire* _wire,
					Member* _from,
					Member* _to)
{	
	Cmd* cmd = new Cmd_ConnectWire(_wire, _from, _to);
	History::global->addAndExecute(cmd);
}

Entity::~Entity()
{
	wires.clear();

	// Delete all components
	for(auto pair : components)
	{
		delete pair.second;
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
				node->setDirty(true);
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

void Nodable::Entity::addWire(Wire* _wire)
{
	wires.push_back(_wire);
}

void Nodable::Entity::removeWire(Wire* _wire)
{
	auto found = std::find(wires.begin(), wires.end(), _wire);
	if(found != wires.end())
		wires.erase(found);
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