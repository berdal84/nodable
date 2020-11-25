#include <algorithm>    // for std::find

#include "Node.h"
#include "Log.h"		// for LOG_DEBUG(...)
#include "NodeView.h"
#include "Wire.h"
#include "WireView.h"
#include "History.h"
#include "DataAccess.h"
#include "ComputeUnaryOperation.h"
#include "ComputeBinaryOperation.h"

using namespace Nodable;

void Node::Disconnect(Wire* _wire)
{
	_wire->getTarget()->setInputMember(nullptr);

	auto targetNode = _wire->getTarget()->getOwner()->as<Node>();	
	auto sourceNode = _wire->getSource()->getOwner()->as<Node>();

	targetNode->removeWire(_wire);
	sourceNode->removeWire(_wire);

	NodeTraversal::SetDirty(targetNode);

    delete _wire;

	return;
}

Wire* Node::Connect( Member* _from, Member* _to)
{
    Wire* wire;

    _to->setInputMember(_from);
    auto targetNode = _to->getOwner()->as<Node>();
    auto sourceNode = _from->getOwner()->as<Node>();

    // Link wire to members
    auto sourceContainer = sourceNode->getParentContainer();

    if (sourceContainer != nullptr)
    {
        wire = sourceContainer->newWire();
    }
    else
    {
        wire = new Wire();
    }

    wire->setSource(_from);
    wire->setTarget(_to);

    targetNode->addWire(wire);
    sourceNode->addWire(wire);

    NodeTraversal::SetDirty(targetNode);

    return wire;
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

	// Delete all components
	for(auto pair : components)
	{
		delete pair.second;
	}

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

Container *Node::getInnerContainer() const
{
    return this->innerContainer;
}

void Node::setInnerContainer(Container *_container)
{
    this->innerContainer = _container;
}

const Operator* Node::getConnectedOperator(const Member *_localMember)
{
    assert(this->has(_localMember));

    const Operator* result{};

    /*
     * Find a wire connected to _member
     */
    auto found = std::find_if(wires.cbegin(),wires.cend(), [_localMember](const Wire* wire)->bool {
        return wire->getTarget() == _localMember;
    });

    /*
     * If found, we try to get the ComputeXXXXXOperator from it's source
     */
    if ( found != wires.end() )
    {
        auto node = (*found)->getSource()->getOwner()->as<Node>();
        // TODO: factorise
        if (auto binOpComponent = node->getComponent<ComputeBinaryOperation>())
        {
            result = binOpComponent->ope;
        }
        else if (auto unaryOpComponent = node->getComponent<ComputeUnaryOperation>())
        {
            result = unaryOpComponent->ope;
        }
    }

    return result;

}

bool Node::hasWireConnectedTo(const Member *_localMember)
{
    /*
     * Find a wire connected to _member
     */
    auto found = std::find_if(wires.cbegin(),wires.cend(), [_localMember](const Wire* wire)->bool {
        return wire->getTarget() == _localMember;
    });

    return found != wires.end();
}

Member* Node::getSourceMemberOf(const Member *_localMember)
{
    auto found = std::find_if(wires.begin(),wires.end(), [_localMember](const Wire* wire)->bool {
        return wire->getTarget() == _localMember;
    });

    return (*found)->getSource();
}
