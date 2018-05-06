#include "Node.h"
#include "Log.h"		// for LOG_DBG(...)
#include <algorithm>    // std::find_if
#include <cstring>      // for strcmp
#include "Node_Value.h"
#include "Node_Number.h"
#include "Node_String.h"
#include "Node_Lexer.h"
#include "Node_Container.h"
#include "Node_Variable.h"
#include "Node_BinaryOperations.h"
#include <imgui.h>
using namespace Nodable;

 // Node :
//////////

ImVec2 Node::s_cameraPosition(0.0f,0.0f);

Node::Node()
{
	LOG_DBG("Node::Node()\n");
	this->inputs  = new Node_Container("inputs", this);
	this->outputs = new Node_Container("outputs", this);
	this->members = new Node_Container("members", this);
}

Node::~Node()
{
	delete inputs;
	delete outputs;
	delete members;
}

void Node::begin()
{
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_ShowBorders | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize;
    //if (no_titlebar)  window_flags |= ImGuiWindowFlags_NoTitleBar;
    //if (!no_border)   window_flags |= ImGuiWindowFlags_ShowBorders;
    //if (no_resize)    window_flags |= ImGuiWindowFlags_NoResize;
    //if (no_move)      window_flags |= ImGuiWindowFlags_NoMove;
    //if (no_scrollbar) window_flags |= ImGuiWindowFlags_NoScrollbar;
    //if (no_collapse)  window_flags |= ImGuiWindowFlags_NoCollapse;
    //if (!no_menu)     window_flags |= ImGuiWindowFlags_MenuBar;

	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, opacity);
	if(opacity < 1.0f)
		opacity += (1.0f - opacity) * 0.05f;
	std::string name = getLabel() + std::string("##") + std::to_string((size_t)this);
	ImGui::SetNextWindowSize(size, ImGuiSetCond_FirstUseEver);
	ImGui::SetNextWindowPos(getPosition(), ImGuiSetCond_FirstUseEver);	
	ImGui::Begin(name.c_str(), &visible, window_flags);

	ImGui::PushItemWidth(150.0f);
}

ImVec2 Node::getPosition()const
{
	return ImVec2(position.x + s_cameraPosition.x, position.y + s_cameraPosition.y);
}

ImVec2 Node::getInputPosition()const
{
	auto pos = getPosition();
	return ImVec2(pos.x, pos.y + size.y * 0.5f);
}

ImVec2 Node::getOutputPosition()const
{
	auto pos = getPosition();
	return ImVec2(pos.x + size.x, pos.y + size.y * 0.5f);
}

void Node::setPosition(ImVec2 _position)
{
	this->position = _position;
}

void Node::end()
{
	ImGui::PopItemWidth();
	ImGui::End();
	ImGui::PopStyleVar();
}

void Node::draw()
{
	if ( !visible)
		return;

	begin();
	setPosition( ImGui::GetWindowPos());

	this->size     = ImGui::GetWindowSize();
	ImGui::Text("%s", label.c_str());
	ImGui::SameLine();
	showDetails ^= ImGui::Button("...", ImVec2(20.0f, 20.0f));
	ImGui::SameLine();
	visible = !ImGui::Button("x", ImVec2(20.0f, 20.0f));

	if (showDetails)
	{
		if(inputs != nullptr)
		{
			if(ImGui::TreeNodeEx("Inputs", ImGuiTreeNodeFlags_Framed))
			{
				inputs->drawLabelOnly();
				ImGui::TreePop();
			}
		}

		if(outputs != nullptr)
		{
			if(ImGui::TreeNodeEx("Outputs", ImGuiTreeNodeFlags_Framed))
			{
				outputs->drawLabelOnly();
				ImGui::TreePop();
			}
		}

		if(members != nullptr)
		{
			if(ImGui::TreeNodeEx("Members", ImGuiTreeNodeFlags_Framed))
			{
				members->drawLabelOnly();
				ImGui::TreePop();
			}
		}
	}
	end();

	// Draw wires to its output
	auto cursorPosBackup = ImGui::GetCursorScreenPos();

	auto out = outputs->getVariables();
	for(auto each : out)
	{
		// draw line
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        draw_list->PushClipRectFullScreen();

        // Compute start and end point
        ImVec2 pos0 = getOutputPosition();     
        ImVec2 pos1 = each->getValueAsNode()->getInputPosition();

        // Compute tangents
        float distX = pos1.x - pos0.x;
        float positiveDistX = distX < 0.0f ? -distX : distX;
        positiveDistX = positiveDistX < 200.0f ? 200.0f : positiveDistX;        

        ImVec2 cp0(pos0.x + positiveDistX*0.5f, pos0.y);
        ImVec2 cp1(pos1.x - positiveDistX*0.5f, pos1.y);

        // draw brzier curve
        ImVec2 arrowPos(pos1.x - 7.0f, pos1.y);
		draw_list->AddBezierCurve(pos0, cp0, cp1, arrowPos, ImColor(1.0f, 1.0f, 1.0f, 1.0f), 2.0f);
		
		float arrowSize = 7.0f;
		draw_list->AddCircleFilled(pos0, 5.0f, ImColor(1.0f, 1.0f, 1.0f, 1.0f));

        draw_list->AddLine(ImVec2(arrowPos.x - arrowSize*2.0f, pos1.y + arrowSize), arrowPos, ImColor(1.0f, 1.0f, 1.0f, 1.0f), 2.0f);
        draw_list->AddLine(ImVec2(arrowPos.x - arrowSize*2.0f, pos1.y - arrowSize), arrowPos, ImColor(1.0f, 1.0f, 1.0f, 1.0f), 2.0f);
        draw_list->AddCircleFilled(pos1, 5.0f, ImColor(1.0f, 1.0f, 1.0f, 1.0f));
        draw_list->PopClipRect();
	}

	ImGui::SetCursorScreenPos(cursorPosBackup);
}

Node_Container* Node::getParent()const
{
	return this->parent;
}

void Node::setParent(Node_Container* _container)
{
	this->parent = _container;
}

Node_Variable* Node::getInput  (const char* _name)const
{
	return inputs->find(_name);
}

Node_Variable* Node::getOutput (const char* _name)const
{
	return outputs->find(_name);
}

Node_Variable* Node::getMember (const char* _name)const
{
	return members->find(_name);
}

void Node::setInput  (Node* _node, const char* _name)
{
	inputs->setSymbol(_name, _node);
}

void Node::setOutput (Node* _node, const char* _name)
{
	outputs->setSymbol(_name, _node);
}

void Node::setMember (Node* _node, const char* _name)
{
	members->setSymbol(_name, _node);
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

void Node::ArrangeRecursive(Node* _node, ImVec2 _position)
{
	if ( _node == nullptr)
		return;	

	_node->setPosition(_position);

	// Draw its inputs
	auto inputs = _node->inputs->getVariables();
	int n = inputs.size();
	for(int i = 0; i < n ; i++ )
	{
		ImVec2 inputPos = _position;
		inputPos.x -= 130.0f;
		if ( n > 1)
			inputPos.y += (float(i)/float(n-1) - 0.5f ) * 150.0f; 

		ArrangeRecursive(inputs[i]->getValueAsNode(), inputPos);
	}
}