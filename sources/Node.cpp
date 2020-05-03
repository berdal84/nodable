#include "Node.h"
#include "Log.h"		// for LOG_DEBUG(...)
#include "NodeView.h"
#include <algorithm>    // for std::find
#include "Wire.h"
#include "WireView.h"
#include "History.h"
#include "DataAccess.h"
#include "BinaryOperation.h"

using namespace Nodable;

void Node::Disconnect(Wire* _wire)
{
	_wire->getTarget()->setInputMember(nullptr);
	_wire->getTarget()->getOwner()->as<Node>()->setDirty();

	_wire->setTarget(nullptr);
	_wire->setSource(nullptr);

	return;
}

void Node::Connect( Wire* _wire,
					Member* _from,
					Member* _to)
{	
	Cmd_ConnectWire command(_wire, _from, _to);
	command.execute();
}

Node::~Node()
{
	wires.clear();

	// Delete all components
	for(auto pair : components)
	{
		delete pair.second;
	}
}

void Node::removeComponent(const std::string& _componentName)
{
	components.erase(_componentName);
}

bool Node::isDirty()const
{
	return dirty;
}

void Node::setDirty(bool _value)
{
	// Propagate thru output wires only if the node is no already dirty.
	// node: if this node is already dirty, all its output should already be dirty too.
	if (!dirty)
	{
		for(auto wire : wires)
		{
			if (wire->getSource()->getOwner() == this && wire->getTarget() != nullptr)
			{
				auto node = reinterpret_cast<Node*>(wire->getTarget()->getOwner());
				node->setDirty(true);
			}
		}
	}

	dirty = _value;
}

Container* Node::getParent()const
{
	return this->parent;
}

void Node::setParent(Container* _container)
{
	this->parent = _container;
}

void Node::setLabel(const char* _label)
{
	this->label = _label;
}

void Node::setLabel(std::string _label)
{
	this->label = _label;
}

const char* Node::getLabel()const
{
	return this->label.c_str();
}

void Nodable::Node::addWire(Wire* _wire)
{
	wires.push_back(_wire);
}

void Nodable::Node::removeWire(Wire* _wire)
{
	auto found = std::find(wires.begin(), wires.end(), _wire);
	if(found != wires.end())
		wires.erase(found);
}

std::vector<Wire*>& Node::getWires()
{
	return wires;
}

int Node::getInputWireCount()const
{
	int count = 0;
	for(auto w : wires)
	{
		if ( w->getTarget()->getOwner() == this)
			count++;
	}
	return count;
}

int Node::getOutputWireCount()const
{
	int count = 0;
	for(auto w : wires)
	{
		if ( w->getSource()->getOwner() == this)
			count++;
	}
	return count;
}

bool Node::update()
{
	bool success = true;

	// Evaluates only if dirty flag is on
	if (isDirty())
	{
		// first we need to evaluate each input and transmit its results thru the wire
		for (auto wire : wires)
		{
			auto wireTarget = wire->getTarget();
			auto wireSource = wire->getSource();

			if ( this->has(wireTarget) &&
				 wireSource != nullptr) 
			{
				/* update the source entity */
				reinterpret_cast<Node*>(wireSource->getOwner())->update();
				
				/* transfert the freshly updated value from source to target member */
				wireTarget->updateValueFromInputMemberValue();
			}
		}

		// then we evaluates this node
		if(hasComponent<BinaryOperationComponent>())
			getComponent<BinaryOperationComponent>()->update();
		
		if(hasComponent<DataAccess>())
			getComponent<DataAccess>()->update();

		setDirty(false);
	}

	return success;
}

void Node::onMemberValueChanged(const char* _name)
{
	setDirty(true);
	updateLabel();	
}