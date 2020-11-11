#include "Container.h"
#include "Log.h"
#include "Parser.h"
#include "Node.h"
#include "Variable.h"
#include "ComputeBinaryOperation.h"
#include "ComputeUnaryOperation.h"
#include "Wire.h"
#include "WireView.h"
#include "DataAccess.h"
#include <cstring>      // for strcmp
#include <algorithm>    // for std::find_if
#include <utility>
#include "NodeView.h"
#include "Application.h"
#include "NodeTraversal.h"
#include <IconFontCppHeaders/IconsFontAwesome5.h>

using namespace Nodable;

ImVec2 Container::LastResultNodePosition = ImVec2(-1, -1); // draft try to store node position

Container::Container(const std::shared_ptr<const Language>& _language):
        language( _language ),
        factory ( std::make_unique<NodeFactory>(_language) )
{

}

Container::~Container()
{
	clear();
}

void Container::clear()
{
    LOG_MESSAGE(1u, "Container::clear() - // start\n");

	// Store the Result node position to restore it later
	if ( resultNode )
	{
		auto view = resultNode->getComponent<NodeView>();
		Container::LastResultNodePosition = view->getRoundedPosition();
	}

	variables.clear();
    nodes.clear();
    resultNode.reset();

    LOG_MESSAGE(1u, "Container::clear() - // end\n");
}

UpdateResult Container::update()
{
    /*
        1 - Delete flagged Nodes
    */
    {
        auto nodeIndex = nodes.size();

        while (nodeIndex > 0)
        {
            nodeIndex--;
            auto node = nodes.at(nodeIndex);

            if (node->needsToBeDeleted())
            {
                remove(node);
            }

        }
    }

	/*
	    2 - Update all Nodes
    */
    size_t updatedNodesCount(0);
    auto result = Result::Success;
    {
        auto it = nodes.begin();

        while (it < nodes.end() && result != Result::Failure)
        {
            auto node = *it;

            if (node && node->isDirty())
            {
                updatedNodesCount++;
                result = NodeTraversal::Update(node);
            }

            ++it;
        }
    }

	if( result != Result::Failure &&
	    updatedNodesCount > 0 && NodeView::GetSelected() != nullptr)
    {
	    return UpdateResult::SuccessWithChanges;
    }
	else
    {
	    return UpdateResult::SuccessWithoutChanges;
    }

}

void Container::add(const std::shared_ptr<Node>& _node)
{
	this->nodes.push_back( _node );
	_node->setParentContainer(this);
}

void Container::remove(const std::shared_ptr<Node>& _node)
{

    if ( auto nodeAsVariable = std::dynamic_pointer_cast<Variable>(_node) )
    {
        auto it = variables.find(nodeAsVariable->getName());
        if (it != variables.end())
        {
            variables.erase(it);
        }
    }

    {
        auto it = std::find(nodes.begin(), nodes.end(), _node);
        if (it != nodes.end())
        {
            nodes.erase(it);
        }
    }

    if (_node == this->resultNode)
    {
        this->resultNode.reset();
    }
}

Variable* Container::findVariable(std::string _name)
{
	auto pair = variables.find(_name);

	if ( pair != variables.end() )
    {
	    return pair->second;
    }

	return nullptr;
}

std::shared_ptr<Variable> Container::newResult()
{
	auto variable = newVariable(ICON_FA_SIGN_OUT_ALT " Result");
    variable->get("value")->setAllowedConnections(Way::In);                     // disable output because THIS node is the output !
	resultNode = variable;
	return variable;
}

std::shared_ptr<Variable> Container::newVariable(const std::string& _name)
{
    auto alreadyExisting = this->findVariable(_name);
    if ( alreadyExisting )
    {
        throw std::runtime_error( "Unable to create a variable because a variable with the same name already exists\n");
    }

	auto node = factory->newVariable(_name);
	this->add(node);
    this->variables.insert_or_assign(_name, node.get());

	return node;
}

std::shared_ptr<Variable> Container::newNumber(double _value)
{
	auto node = factory->newNumber(_value);
	this->add(node);
	return node;
}

std::shared_ptr<Variable> Container::newNumber(const char* _value)
{
	auto node = factory->newNumber( std::stod(_value) );
	this->add(node);
	return node;
}

std::shared_ptr<Variable> Container::newString(const char* _value)
{
	auto node = factory->newString(_value);
	this->add(node);
	return node;
}

std::shared_ptr<Node> Container::newBinOp(std::shared_ptr<const Operator> _operator)
{
	auto node = factory->newBinOp(std::move(_operator));
	this->add(node);
	return node;
}

std::shared_ptr<Node> Container::newUnaryOp(std::shared_ptr<const Operator> _operator)
{
	auto node = factory->newUnaryOp(std::move(_operator));
	this->add(node);
	return node;
}

std::shared_ptr<Node> Container::newFunction(std::shared_ptr<const Function> _function)
{
	auto node = factory->newFunction(std::move(_function));
	this->add(node);
	return node;
}


std::shared_ptr<Wire> Container::newWire()
{
	return factory->newWire();
}

void Container::tryToRestoreResultNodePosition()
{
    if ( resultNode )
    {
        // Store the Result node position to restore it later
        auto nodeView = resultNode->getComponent<NodeView>();
        bool resultNodeHadPosition = Container::LastResultNodePosition.x != -1 &&
                                     Container::LastResultNodePosition.y != -1;

        if (nodeView && this->hasComponent<View>())
        {
            auto view = this->getComponent<View>();

            if (resultNodeHadPosition)
            {                                 /* if result node had a position stored, we restore it */
                nodeView->setPosition(Container::LastResultNodePosition);
            }

            if (!NodeView::IsInsideRect(nodeView, view->visibleRect))
            {
                ImVec2 defaultPosition = view->visibleRect.GetCenter();
                defaultPosition.x += view->visibleRect.GetWidth() * 1.0f / 6.0f;
                nodeView->setPosition(defaultPosition);
            }
        }
    }
    else
    {
        LOG_WARNING(0u, "Unable to restore result node position. resultNode is not set.\n" );
    }
}

size_t Container::getNodeCount()const
{
	return nodes.size();
}
