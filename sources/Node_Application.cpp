#include "Node_Application.h"
#include <iostream>
#include <string>
#include "Nodable.h" 	// for NODABLE_VERSION
#include "Log.h" 		// for LOG_DBG
#include "Node.h"
#include "Node_String.h"
#include "Node_Lexer.h"
#include "Node_Container.h"
#include <unistd.h>
#include <imgui.h>

using namespace Nodable;

Node_Application::Node_Application()
{

}

Node_Application::~Node_Application()
{

}

void Node_Application::init()
{
	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_Text]                  = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_TextDisabled]          = ImVec4(0.21f, 0.21f, 0.21f, 1.00f);
	style.Colors[ImGuiCol_WindowBg]              = ImVec4(0.49f, 0.63f, 0.69f, 1.00f);
	style.Colors[ImGuiCol_ChildWindowBg]         = ImVec4(1.00f, 1.00f, 1.00f, 0.08f);
	style.Colors[ImGuiCol_PopupBg]               = ImVec4(0.66f, 0.66f, 0.66f, 1.00f);
	style.Colors[ImGuiCol_Border]                = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	style.Colors[ImGuiCol_BorderShadow]          = ImVec4(0.30f, 0.30f, 0.30f, 0.50f);
	style.Colors[ImGuiCol_FrameBg]               = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
	style.Colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.90f, 0.80f, 0.80f, 1.00f);
	style.Colors[ImGuiCol_FrameBgActive]         = ImVec4(0.90f, 0.65f, 0.65f, 1.00f);
	style.Colors[ImGuiCol_TitleBg]               = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
	style.Colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.49f, 0.63f, 0.69f, 1.00f);
	style.Colors[ImGuiCol_TitleBgActive]         = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
	style.Colors[ImGuiCol_MenuBarBg]             = ImVec4(0.60f, 0.60f, 0.60f, 0.98f);
	style.Colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.61f, 0.61f, 0.62f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.70f, 0.70f, 0.70f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.80f, 0.80f, 0.80f, 1.00f);
	style.Colors[ImGuiCol_ComboBg]               = ImVec4(0.65f, 0.65f, 0.65f, 0.99f);
	style.Colors[ImGuiCol_CheckMark]             = ImVec4(0.90f, 0.90f, 0.90f, 0.50f);
	style.Colors[ImGuiCol_SliderGrab]            = ImVec4(0.71f, 0.46f, 0.22f, 0.63f);
	style.Colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.71f, 0.46f, 0.22f, 1.00f);
	style.Colors[ImGuiCol_Button]                = ImVec4(0.89f, 0.66f, 0.27f, 0.63f);
	style.Colors[ImGuiCol_ButtonHovered]         = ImVec4(0.98f, 0.73f, 0.29f, 0.95f);
	style.Colors[ImGuiCol_ButtonActive]          = ImVec4(0.80f, 0.50f, 0.50f, 1.00f);
	style.Colors[ImGuiCol_Header]                = ImVec4(0.70f, 0.70f, 0.70f, 1.00f);
	style.Colors[ImGuiCol_HeaderHovered]         = ImVec4(0.84f, 0.84f, 0.84f, 0.96f);
	style.Colors[ImGuiCol_HeaderActive]          = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	style.Colors[ImGuiCol_Column]                = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	style.Colors[ImGuiCol_ColumnHovered]         = ImVec4(0.70f, 0.60f, 0.60f, 1.00f);
	style.Colors[ImGuiCol_ColumnActive]          = ImVec4(0.90f, 0.70f, 0.70f, 1.00f);
	style.Colors[ImGuiCol_ResizeGrip]            = ImVec4(1.00f, 1.00f, 1.00f, 0.30f);
	style.Colors[ImGuiCol_ResizeGripHovered]     = ImVec4(1.00f, 1.00f, 1.00f, 0.60f);
	style.Colors[ImGuiCol_ResizeGripActive]      = ImVec4(1.00f, 1.00f, 1.00f, 0.90f);
	style.Colors[ImGuiCol_CloseButton]           = ImVec4(0.44f, 0.44f, 0.44f, 1.00f);
	style.Colors[ImGuiCol_CloseButtonHovered]    = ImVec4(0.75f, 0.75f, 0.75f, 1.00f);
	style.Colors[ImGuiCol_CloseButtonActive]     = ImVec4(0.89f, 0.89f, 0.89f, 1.00f);
	style.Colors[ImGuiCol_PlotLines]             = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	style.Colors[ImGuiCol_PlotLinesHovered]      = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogram]         = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.00f, 0.00f, 1.00f, 0.35f);
	style.Colors[ImGuiCol_ModalWindowDarkening]  = ImVec4(0.20f, 0.20f, 0.20f, 0.55f);

	style.FrameRounding = 3.0f;
	style.AntiAliasedShapes = true;
	style.AntiAliasedLines = true;

	LOG_MSG(" -- Nodable v%s - by Berdal84 - 2017 --\n", NODABLE_VERSION);

	// Create a context	
	this->ctx    = new Node_Container("Global");
	LOG_MSG("Launching the command line interpreter...\n");

	// Create few nodes to identify keywords :
	this->exitString 		= ctx->createNodeString("exit"); 	/* define the Node_String "exit" as a keyword to exit application.*/
}

void Node_Application::draw()
{
	this->ctx->draw();
}

bool Node_Application::eval(std::string _expression)
{
	LOG_MSG("Node_Application::eval()");
	bool quit;
	this->lastString 		= ctx->createNodeString(_expression.c_str());

	if ( lastString->isEqualsTo(exitString) ){
		quit = true;		
	}else{
		if ( !lastString->isEmpty())
		{
			/* Create a Lexer node. The lexer will cut expression string into tokens
			(ex: "2*3" will be tokenized as : number"->"2", "operator"->"*", "number"->"3")*/
			auto lexer = ctx->createNodeLexer(lastString);
			lexer->evaluate();
			//delete lexer;
		}
		quit = false;
	}	

	return quit;
}

void Node_Application::shutdown()
{
	LOG_MSG("Shutdown Nodable...\n");

	// Free memory
	delete this->exitString;
	delete this->ctx;
	delete this->lastString;
}

