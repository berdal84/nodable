#include "Node_Container.h"
#include "Log.h"
#include "Node_Lexer.h"
#include "Node.h"
#include "Node_Variable.h"
#include "Node_BinaryOperations.h"
#include "Wire.h"
#include "WireView.h"

#include <cstring>      // for strcmp
#include <algorithm>    // for std::find_if
#include "NodeView.h"

using namespace Nodable;

Node_Container::Node_Container(const char* _name, Node* _parent):
name(_name),
parent(_parent)
{
	LOG_DBG("A new container named %s' has been created.\n", _name);
}

void Node_Container::clear()
{
	//for (auto each : nodes)
	//0	delete each;
	nodes.resize(0);
	variables.resize(0);
}

void Node_Container::frameAll()
{

}

void Node_Container::draw()
{
	bool isAnyItemDragged = false;
	bool isAnyItemHovered = false;

	

	// 1 - Draw nodes
	for(auto each : this->nodes)
	{
		if ( each != nullptr)
		{
			auto view = each->getView();

			if (view != nullptr)
			{
				view->draw();
				isAnyItemDragged |= NodeView::GetDragged() == view;
				isAnyItemHovered |= view->isHovered();
			}
		}
	}

	


	// 2 - Draw wires
	for(auto each : this->nodes)
	{
		if ( each != nullptr)
		{
			auto wires = each->getWires();

			for(auto eachWire : wires)
			{
				eachWire->transmitData();
				eachWire->getView()->draw();

			}
		}
	}

	auto selectedView = NodeView::GetSelected();
	// Deselection
	if( !isAnyItemHovered && ImGui::IsMouseClicked(0) && ImGui::IsWindowFocused())
		NodeView::SetSelected(nullptr);

	
	if (selectedView != nullptr)
	{
		// Deletion
		if ( ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Delete)))
			selectedView->setVisible(false);
		// Arrange 
		else if ( ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_A)))
			selectedView->arrange();
	}

	// Draft Mouse PAN
	if( ImGui::IsMouseDragging() && ImGui::IsWindowFocused() && !isAnyItemDragged )
	{
		auto drag = ImGui::GetMouseDragDelta();
		for(auto each : this->nodes)
		{
			if (each != nullptr){
				auto view = each->getView();

				if (view != nullptr)
					each->getView()->translate(drag);
			}
		}
		ImGui::ResetMouseDragDelta();
	}
}

void Node_Container::drawLabelOnly()
{
	{
		for(auto each : this->nodes)
		{
			if (auto symbol = dynamic_cast<Node_Variable*>(each))
				ImGui::Text("%s => %s", symbol->getName(), symbol->getValueAsString().c_str());
		}
	}
}

void Node_Container::addNode(Node* _node)
{
	/* Add the node to the node vector list*/
	this->nodes.push_back(_node);

	/* Set the node's container to this */
	_node->setParent(this);

	LOG_DBG("A node has been added to the container '%s'\n", this->getName());
}

void Node_Container::destroyNode(Node* _node)
{
	{
		auto it = std::find(variables.begin(), variables.end(), _node);
		if (it != variables.end())
			variables.erase(it);
	}

	{
		auto it = std::find(nodes.begin(), nodes.end(), _node);
		if (it != nodes.end())
			nodes.erase(it);
	}

	delete _node;
}

Node_Variable* Node_Container::find(const char* _name)
{
	if ( _name == '\0')
		return nullptr;

	LOG_DBG("Searching node '%s' in container '%s' : ", _name, this->getName());

	auto findFunction = [_name](const Node_Variable* _node ) -> bool
	{
		return strcmp(_node->getName(), _name) == 0;
	};

	auto it = std::find_if(variables.begin(), variables.end(), findFunction);
	if (it != variables.end()){
		LOG_DBG("FOUND !\n");
		return *it;
	}
	LOG_DBG("NOT found...\n");
	return nullptr;
}

Node_Variable* Node_Container::createNodeVariable(const char* _name)
{
	auto variable = new Node_Variable();
	variable->setName(_name);
	this->variables.push_back(variable);
	addNode(variable);
	return variable;
}

Node_Variable*          Node_Container::createNodeNumber(int _value)
{
	auto node = new Node_Variable();
	node->setValue(_value);
	addNode(node);
	return node;
}

Node_Variable*          Node_Container::createNodeNumber(const char* _value)
{
	auto node = new Node_Variable();
	node->setValue(std::stod(_value));
	addNode(node);
	return node;
}

Node_Variable*          Node_Container::createNodeString(const char* _value)
{
	auto node = new Node_Variable();
	node->setValue(_value);
	addNode(node);
	return node;
}


Node_BinaryOperation* Node_Container::createNodeBinaryOperation(std::string _op, Node_Variable* _leftInput, Node_Variable* _rightInput, Node_Variable* _output)
{
	Node_BinaryOperation* node;

	if      ( _op == "+")
		node = createNodeAdd();
	else if ( _op == "-")
		node = createNodeSubstract();
	else if (_op =="*")
		node = createNodeMultiply();
	else if ( _op == "/")
		node = createNodeDivide();
	else if ( _op == "=")
		node = createNodeAssign();
	else
		return nullptr;

	Node::Connect(_leftInput,  node,   "value", "left");
	Node::Connect(_rightInput, node,   "value", "right");
	Node::Connect(node,        _output,"result", "value");

	return node;
}


Node_Add* Node_Container::createNodeAdd()
{
	auto node = new Node_Add();
	addNode(node);
	return node;
}

Node_Substract* Node_Container::createNodeSubstract()
{
	auto node = new Node_Substract();
	addNode(node);
	return node;
}

Node_Multiply* Node_Container::createNodeMultiply()
{
	auto node = new Node_Multiply();
	addNode(node);
	return node;
}

Node_Divide* Node_Container::createNodeDivide()
{
	auto node = new Node_Divide();
	addNode(node);
	return node;
}

Node_Assign* Node_Container::createNodeAssign()
{
	auto node = new Node_Assign();
	addNode(node);
	return node;
}


Node_Lexer* Node_Container::createNodeLexer(Node_Variable* _input)
{
	Node_Lexer* lexer = new Node_Lexer();
	Node::Connect(_input, lexer, "value", "expression");
	addNode(lexer);
	return lexer;
}

const char* Node_Container::getName()const
{
	return name.c_str();
}

size_t Node_Container::getSize()const
{
	return nodes.size();
}