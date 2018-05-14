#include "Node_Application.h"
#include "Nodable.h" 	// for NODABLE_VERSION
#include "Log.h" 		// for LOG_DBG
#include "Node_Lexer.h"
#include "Node_Container.h"
#include "ApplicationView.h"
#include "Node_Variable.h"

#include <unistd.h>
#include <imgui.h>

using namespace Nodable;

Node_Application::Node_Application(const char* _name)
{
	this->view = std::unique_ptr<ApplicationView>(new ApplicationView(_name, this));
}

Node_Application::~Node_Application()
{

}

bool Node_Application::init()
{
	this->ctx = std::unique_ptr<Node_Container>(new Node_Container("Global"));
	return view->init();;
}

void Node_Application::clear()
{
	this->ctx.get()->clear();
}

bool Node_Application::update()
{
	return !quit;
}

void Node_Application::draw()
{
	view->draw();
}

void Node_Application::stopExecution()
{
	quit = true;
}

bool Node_Application::eval(std::string _expression)
{
	LOG_MSG("Node_Application::eval() - create a variable.\n");
	lastString = ctx->createNodeVariable("Command");

	LOG_DBG("Node_Lexer::eval() - assign the expression string to that variable\n");
	lastString->setValue(_expression.c_str());

	LOG_DBG("Node_Lexer::eval() - check if users type the exit keyword.\n");
	if ( lastString->getValueAsString() == "exit" ){
		LOG_DBG("Node_Lexer::eval() - stopExecution...\n");
		stopExecution();		
	}else{
		LOG_DBG("Node_Lexer::eval() - check if expression is not empty\n");
		if ( lastString->isSet())
		{
			/* Create a Lexer node. The lexer will cut expression string into tokens
			(ex: "2*3" will be tokenized as : number"->"2", "operator"->"*", "number"->"3")*/
			LOG_DBG("Node_Lexer::eval() - create a lexer with the expression string\n");
			auto lexer = ctx->createNodeLexer(lastString);
			return lexer->eval();
			//ctx->destroyNode(lexer);
		}
	}	

	return false;
}

void Node_Application::shutdown()
{
	LOG_MSG("Shutdown Nodable...\n");

	// Free memory
	delete this->lastString;
}

Node_Container* Node_Application::getContext()const
{
	return this->ctx.get();
}

