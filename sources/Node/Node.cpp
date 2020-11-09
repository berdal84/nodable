#include <algorithm>    // for std::find

#include "Node.h"
#include "Log.h"		// for LOG_DEBUG(...)
#include "NodeView.h"
#include "Wire.h"
#include "WireView.h"
#include "History.h"
#include "DataAccess.h"
#include "ComputeBase.h"
#include "NodeTraversal.h"

using namespace Nodable;

void Node::Disconnect(std::shared_ptr<Wire> _wire)
{
	_wire->getTarget()->setInputMember(nullptr);

	auto targetNode = _wire->getTarget()->getOwner()->as<Node>();	
	auto sourceNode = _wire->getSource()->getOwner()->as<Node>();

	targetNode->removeWire(_wire);
	sourceNode->removeWire(_wire);

	NodeTraversal::SetDirty(targetNode);
}

std::shared_ptr<Wire> Node::Connect(std::shared_ptr<Member> _from, std::shared_ptr<Member> _to)
{	
	auto command = std::make_unique<Cmd_ConnectWire>(_from, _to);
	command->execute();
	return command->getWire();
}

Node::Node(std::string _label):

	parentContainer(nullptr),
	label(_label),
	dirty(true)
{

}

Node::~Node()
{
    // Disconnect and clear wires
    std::for_each(wires.crbegin(), wires.crend(), [](auto item) {
       Node::Disconnect(item);
    });
}

bool Node::isDirty()const
{
	return dirty;
}

void Node::setDirty(bool _value)
{
	dirty = _value;
}

Container* Node::getParentContainer()const
{
	return this->parentContainer;
}

void Node::setParentContainer(Container* _container)
{
	this->parentContainer = _container;
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

void Nodable::Node::addWire(std::shared_ptr<Wire> _wire)
{
	wires.push_back(_wire);
}

void Nodable::Node::removeWire(std::shared_ptr<Wire> _wire)
{
	auto found = std::find(wires.begin(), wires.end(), _wire);
	if(found != wires.end())
		wires.erase(found);
}

std::vector<std::shared_ptr<Wire>>& Node::getWires()
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

UpdateResult Node::update()
{
    // TODO: take in account the result of component's update()

	if(hasComponent<ComputeBase>())
    {
        getComponent<ComputeBase>()->update();
    }

	if(hasComponent<DataAccess>())
    {
        getComponent<DataAccess>()->update();
    }

	return UpdateResult::SuccessWithChanges;
}

void Node::onMemberValueChanged(const char* _name)
{	
	updateLabel();
	NodeTraversal::SetDirty(this);
}