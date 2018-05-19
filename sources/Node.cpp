#include "Node.h"
#include "Log.h"		// for LOG_DBG(...)
#include "NodeView.h"
#include <algorithm>    // for std::find
#include "Wire.h"
#include "WireView.h"

using namespace Nodable;

void Node::Disconnect(Wire* _wire)
{
	auto sourceNode = _wire->getSource();
	auto targetNode = _wire->getTarget();

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

	delete _wire;
}

void Node::Connect(	Node* _from, 
					Node* _to, 
					const char* _fromOutputName, 
					const char* _toInputName)
{
	auto wire = new Wire();
	wire->addComponent("view", new WireView(wire));

	// Connect wire's source and target to nodes _from and _to.
	wire->setSource(_from , _fromOutputName);
	wire->setTarget(_to   , _toInputName);

	// Add this wire to each node. They need to know that they are linked together.
	_from->wires.push_back(wire);
	_to->wires.push_back(wire);
}

Node::Node()
{
	LOG_DBG("Node::Node()\n");	
}

Node::~Node()
{
	for(auto wire : wires)
		Node::Disconnect(wire);

}

void Node::addComponent(const std::string&  _componentName, Node*  _component)
{
	components[_componentName] = _component;
}

bool Node::hasComponent(const std::string&  _componentName)const
{
	auto it = components.find(_componentName);
	return it != components.end();
}

Node* Node::getComponent(const std::string&  _componentName)const
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
			if (wire->getSource() == this && wire->getTarget() != nullptr)
				wire->getTarget()->setDirty(_value);
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

const Members&   Node::getMembers      ()const
{
	return members;
}

Value* Node::getMember (const char* _name)const
{
	return members.at(std::string(_name));
}

Value* Node::getMember (const std::string& _name)const
{
	return members.at(_name.c_str());
}

void Node::addMember (const char* _name, Type_ _type)
{
	members[std::string(_name)] =  new Value(_type);
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
		if ( w->getTarget() == this)
			count++;
	}
	return count;
}

int Node::getOutputWireCount()const
{
	int count = 0;
	for(auto w : wires)
	{
		if ( w->getSource() == this)
			count++;
	}
	return count;
}

bool Node::eval()
{
	return true;
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
			if ( wire->getTarget() == this && wire->getSource() != nullptr)
			{
				wire->getSource()->update();
				wire->transmitData();
			}
		}

		// then we evaluates this node
		this->eval();
		setDirty(false);
	}

	return success;
}