#include "Node.h"
#include "Log.h"		// for LOG_DBG(...)
#include "NodeView.h"
#include <algorithm>    // for std::find

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
}

Node::Node()
{
	LOG_DBG("Node::Node()\n");
	this->view    = new NodeView(this);
}

Node::~Node()
{
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
	members.emplace(std::string(_name), new Value(_type));
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

bool Node::eval()
{
	return true;
}

void Node::update()
{
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

		// the we evaluates this node
		this->eval();
		setDirty(false);
	}
}