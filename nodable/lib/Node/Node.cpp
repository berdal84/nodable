#include <algorithm>    // for std::find
#include <utility>
#include "Nodable.h"
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
        m_props(this),
        m_parentGraph(nullptr),
        m_parent(nullptr),
        m_label(std::move(_label)),
        m_innerGraph(nullptr),
        m_dirty(false),
        m_deletedFlag(false),
        m_previousMaxCount(0),
        m_nextMaxCount(0)
{
//    add("activator", Visibility::Always, Type_Boolean, Way::Way_In);
}

bool Node::isDirty()const
{
    return m_dirty;
}

void Node::setDirty(bool _value)
{
    m_dirty = _value;
}

void Node::setLabel(const char* _label)
{
	this->m_label = _label;
}

void Node::setLabel(std::string _label)
{
	this->m_label = _label;
}

const char* Node::getLabel()const
{
	return this->m_label.c_str();
}

void Nodable::Node::addWire(Wire* _wire)
{
	m_wires.push_back(_wire);
    GraphTraversal::TraverseAndSetDirty(this);
}

void Nodable::Node::removeWire(Wire* _wire)
{
	auto found = std::find(m_wires.begin(), m_wires.end(), _wire);
	if(found != m_wires.end())
		m_wires.erase(found);
}

std::vector<Wire*>& Node::getWires()
{
	return m_wires;
}

int Node::getInputWireCount()const
{
	int count = 0;
	for(auto w : m_wires)
	{
		if ( w->getTarget()->getOwner() == this)
			count++;
	}
	return count;
}

int Node::getOutputWireCount()const
{
	int count = 0;
	for(auto w : m_wires)
	{
		if ( w->getSource()->getOwner() == this)
			count++;
	}
	return count;
}

bool Node::eval() const
{
    // read data from inputs
    for(auto& eachNameToMemberPair : m_props.getMembers())
    {
        Member* eachMember = eachNameToMemberPair.second;
        if( Member* input = eachMember->getInputMember() )
        {
            // transfer value:  input ----> X
            eachMember->setType(input->getType()); // dynamic type, TODO: show a warning ?
            eachMember->set(input);
        }
    }

    // update
    if(hasComponent<ComputeBase>())
    {
        return getComponent<ComputeBase>()->update();
    }
    return true;
}

UpdateResult Node::update()
{
	if(hasComponent<DataAccess>())
    {
        getComponent<DataAccess>()->update();
    }

    this->m_dirty = false;

	return UpdateResult::Success;
}

GraphNode *Node::getInnerGraph() const
{
    return this->m_innerGraph;
}

void Node::setInnerGraph(GraphNode *_graph)
{
    this->m_innerGraph = _graph;
}

const Operator* Node::getConnectedOperator(const Member *_localMember)
{
    assert(m_props.has(_localMember));

    const Operator* result{};

    /*
     * Find a wire connected to _member
     */
    auto found = std::find_if(m_wires.cbegin(), m_wires.cend(), [_localMember](const Wire* wire)->bool {
        return wire->getTarget() == _localMember;
    });

    /*
     * If found, we try to get the ComputeXXXXXOperator from it's source
     */
    if (found != m_wires.end() )
    {
        auto node = (*found)->getSource()->getOwner()->as<Node>();
        // TODO: factorise
        if (auto binOpComponent = node->getComponent<ComputeBinaryOperation>())
        {
            result = binOpComponent->getOperator();
        }
        else if (auto unaryOpComponent = node->getComponent<ComputeUnaryOperation>())
        {
            result = unaryOpComponent->getOperator();
        }
    }

    return result;

}

bool Node::hasWireConnectedTo(const Member *_localMember)
{
    /*
     * Find a wire connected to _member
     */
    auto found = std::find_if(m_wires.cbegin(), m_wires.cend(), [_localMember](const Wire* wire)->bool {
        return wire->getTarget() == _localMember;
    });

    return found != m_wires.end();
}

Member* Node::getSourceMemberOf(const Member *_localMember)
{
    auto found = std::find_if(m_wires.begin(), m_wires.end(), [_localMember](const Wire* wire)->bool {
        return wire->getTarget() == _localMember;
    });

    return (*found)->getSource();
}

void Node::addChild(Node *_node)
{
    auto found = std::find(m_children.begin(), m_children.end(), _node);
    NODABLE_ASSERT(found == m_children.end()); // check if node is not already child
    m_children.push_back(_node);
}

void Node::removeChild(Node *_node)
{
    auto found = std::find(m_children.begin(), m_children.end(), _node);
    NODABLE_ASSERT(found != m_children.end()); // check if node is found before to erase.
    m_children.erase(found);
}

void Node::setParent(Node *_node)
{
    NODABLE_ASSERT(_node != nullptr || this->m_parent != nullptr);
    this->m_parent = _node;
}

void Node::setParentGraph(GraphNode *_parentGraph)
{
    NODABLE_ASSERT(this->m_parentGraph == nullptr); // TODO: implement parentGraph switch
    this->m_parentGraph = _parentGraph;
}

void Node::addInput(Node* _node)
{
    this->m_inputs.push_back(_node);
}

void Node::addOutput(Node *_node)
{
    this->m_outputs.push_back(_node);
}

void Node::removeOutput(Node *_node)
{
    auto found = std::find(m_outputs.begin(), m_outputs.end(), _node);
    NODABLE_ASSERT(found != m_outputs.end()); // check if node is found before to erase.
    m_outputs.erase(found);
}

void Node::removeInput(Node *_node)
{
    auto found = std::find(m_inputs.begin(), m_inputs.end(), _node);
    NODABLE_ASSERT(found != m_inputs.end()); // check if node is found before to erase.
    m_inputs.erase(found);
}

std::vector<Node*>& Node::getInputs() {
    return this->m_inputs;
}

std::vector<Node *>& Node::getOutputs() {
    return this->m_outputs;
}

void Node::setShortLabel(const char *_label) {
    this->m_shortLabel = _label;
}

const char* Node::getShortLabel() const {
    return this->m_shortLabel.c_str();
}

Node::~Node()
{
    if ( !m_components.empty())
        deleteComponents();
}

size_t Node::getComponentCount() const
{
    return m_components.size();
}

size_t Node::deleteComponents()
{
    size_t count(m_components.size());
    for ( const auto& keyComponentPair : m_components)
    {
        delete keyComponentPair.second;
    }
    m_components.clear();
    return count;
}

void Node::removeNext(Node *_node)
{
    auto found = std::find(m_next.begin(), m_next.end(), _node);
    m_next.erase(found);
}

void Node::removePrev(Node *_node)
{
    auto found = std::find(m_previous.begin(), m_previous.end(), _node);
    m_previous.erase(found);
}

void Node::addPrev(Node *_node)
{
    NODABLE_ASSERT(m_previous.size() < m_previousMaxCount);
    m_previous.push_back(_node );
}

void Node::addNext(Node *_node)
{
    NODABLE_ASSERT(m_next.size() < m_nextMaxCount);
    m_next.push_back(_node );
}