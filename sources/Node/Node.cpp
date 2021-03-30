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

Node::Node(std::string _label):

        parentGraph(nullptr),
        parent(nullptr),
        label(_label),
        dirty(true)
{
//    add("activator", Visibility::Always, Type::Boolean, Way::Way_In);
}

Node::~Node()
{
	// Delete all components
	for(auto pair : components)
	{
		delete pair.second;
	}
}

bool Node::isDirty(bool _checkChildren)const
{
    if (_checkChildren)
    {
        NodeTraversal traversal;
        return traversal.hasAChildDirty(this);
    }

    return dirty;
}

void Node::setDirty(bool _value)
{
	dirty = _value;
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
    NodeTraversal traversal;
    traversal.setDirty(this);
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

    this->dirty = false;

	return UpdateResult::Success;
}

void Node::onMemberValueChanged(const char* _name)
{	
	updateLabel();
    NodeTraversal traversal;
    traversal.setDirty(this);
}

GraphNode *Node::getInnerGraph() const
{
    return this->innerGraph;
}

void Node::setInnerGraph(GraphNode *_graph)
{
    this->innerGraph = _graph;
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

void Node::addChild(Node *_node)
{
    auto found = std::find(children.begin(), children.end(), _node);
    NODABLE_ASSERT(found == children.end()); // check if node is not already child
    this->children.push_back(_node);
}

void Node::removeChild(Node *_node)
{
    auto found = std::find(children.begin(), children.end(), _node);
    NODABLE_ASSERT(found != children.end()); // check if node is found before to erase.
    children.erase(found);
}

void Node::setParent(Node *_node)
{
    NODABLE_ASSERT(_node != nullptr || this->parent != nullptr);
    this->parent = _node;
}

void Node::setParentGraph(GraphNode *_parentGraph)
{
    NODABLE_ASSERT(this->parentGraph == nullptr); // TODO: implement parentGraph switch
    this->parentGraph = _parentGraph;
}

void Node::addInput(Node* _node)
{
    this->inputs.push_back(_node);
}

void Node::addOutput(Node *_node)
{
    this->outputs.push_back(_node);
}

void Node::removeOutput(Node *_node)
{
    auto found = std::find(outputs.begin(), outputs.end(), _node);
    NODABLE_ASSERT(found != outputs.end()); // check if node is found before to erase.
    outputs.erase(found);
}

void Node::removeInput(Node *_node)
{
    auto found = std::find(inputs.begin(), inputs.end(), _node);
    NODABLE_ASSERT(found != inputs.end()); // check if node is found before to erase.
    inputs.erase(found);
}

std::vector<Node*>& Node::getInputs() {
    return this->inputs;
}

std::vector<Node *>& Node::getOutputs() {
    return this->outputs;
}

void Node::setShortLabel(const char *_label) {
    this->shortLabel = _label;
}

const char* Node::getShortLabel() const {
    return this->shortLabel.c_str();
}

