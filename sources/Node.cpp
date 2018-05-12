#include "Node.h"
#include "Log.h"		// for LOG_DBG(...)
#include "Node_Container.h"
#include "Node_Variable.h"
#include "Node_Value.h"
#include "NodeView.h"
#include <algorithm> // for std::find
#include "Wire.h"
#include <string> // for strcmp
using namespace Nodable;

void Node::Connect(	Node* _from, 
					Node* _to, 
					const char* _fromOutputName, 
					const char* _toInputName)
{

	// Create an empty wire
	auto wire = new Wire();

	// Connect wire's source and target to nodes _from and _to.
	wire->setSource(_from , _fromOutputName);
	wire->setTarget(_to   , _toInputName);

	// Add this wire to each node. They need to know that they are linked by a wire.
	_from->addWire(wire);
	_to->addWire(wire);

	// force wire to transmit data
	wire->transmitData();
}

Node::Node()
{
	LOG_DBG("Node::Node()\n");
	this->view    = new NodeView(this);
}

Node::~Node()
{
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

const Node_Value& Node::getMember (const char* _name)const
{
	return members.at(std::string(_name));
}

const Node_Value& Node::getMember (const std::string& _name)const
{
	return members.at(_name.c_str());
}

void Node::addMember (const char* _name, Type_ _type)
{
	auto& m = members[std::string(_name)];
	m.setType(_type);
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

NodeView* Node::getView()const
{
	return this->view;
}

void Node::updateWires()
{
	/*
	// both sides
	for (auto wire : wires)
		wire->transmitData();
	*/

	
	// outputs only
	for (auto wire : wires)
	{
		if ( wire->getSource() == this)
			wire->transmitData();
	}
		
}

void Node::addWire(Wire* _wire)
{
	wires.push_back(_wire);
}

void Node::removeWire(Wire* _wire)
{
	auto found = std::find(wires.begin(), wires.end(), _wire);

	if (found != wires.end())
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

bool Node::evaluate()
{
		// outputs only
	for (auto wire : wires)
	{
		if ( wire->getTarget() == this)
			wire->transmitData();
	}
	return true;
}