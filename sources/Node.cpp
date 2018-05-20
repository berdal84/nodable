#include "Node.h"
#include "Log.h"		// for LOG_DBG(...)
#include "NodeView.h"
#include <algorithm>    // for std::find
#include "Wire.h"
#include "WireView.h"

using namespace Nodable;

void Node::Disconnect(Wire* _wire)
{
	auto sourceNode = _wire->getSource()->getOwner()->as<Node*>();
	auto targetNode = _wire->getTarget()->getOwner()->as<Node*>();


	// remove wire pointer from sourceNode's wires.
	{
		auto found = std::find(sourceNode->wires.begin(), sourceNode->wires.end(), _wire);
		sourceNode->wires.erase(found); // I do not check if node has been found, because it should. If not, I prefer to crash here.
	}

	// remove wire pointer from targetNode's wires.
	{
		auto found = std::find(targetNode->wires.begin(), targetNode->wires.end(), _wire);
		targetNode->wires.erase(found); // I do not check if node has been found, because it should. If not, I prefer to crash here.
	}

	_wire->setTarget(nullptr);
	_wire->setSource(nullptr);

	return;

}

void Node::Connect( Wire* _wire,
					Value* _from,
					Value* _to)
{	
	_wire->addComponent("view", new WireView(_wire));

	// Connect wire's source and target to nodes _from and _to.
	_wire->setSource(_from);
	_wire->setTarget(_to);

	_from->getOwner()->as<Node*>()->wires.push_back(_wire);
	_to->getOwner()->as<Node*>()->wires.push_back(_wire);
}

Node::~Node()
{
	for(auto wire : wires)
	{
		Node::Disconnect(wire);
		delete wire;
	}

}

void Node::addComponent(const std::string&  _componentName, Component*  _component)
{
	components[_componentName] = _component;
}

bool Node::hasComponent(const std::string&  _componentName)const
{
	auto it = components.find(_componentName);
	return it != components.end();
}

Component* Node::getComponent(const std::string&  _componentName)const
{
	return components.at(_componentName);
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
				auto node = (Node*)wire->getTarget()->getOwner();
				node->setDirty(_value);
			}
		}
	}

	dirty = _value;
}

Node_Container* Node::getParent()const
{
	return this->parent;
}

void Node::setParent(Node_Container* _container)
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
			if ( wire->getTarget()->getOwner() == this && wire->getSource() != nullptr)
			{
				auto source = (Node*)wire->getSource()->getOwner();
				source->update();
				wire->transmitData();
			}
		}

		// then we evaluates this node
		if(hasComponent("operation"))
			getComponent("operation")->update();

		setDirty(false);
	}

	return success;
}

void Node::onMemberValueChanged(const char* _name)
{
	setDirty(true);
	updateLabel();	
}