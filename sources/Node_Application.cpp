#include "Node_Application.h"
#include "Nodable.h" 	// for NODABLE_VERSION
#include "Log.h" 		// for LOG_DBG
#include "Node_String.h"
#include "Node_Lexer.h"
#include "Node_Container.h"
#include "ApplicationView.h"
#include <unistd.h>
#include <imgui.h>

using namespace Nodable;

Node_Application::Node_Application(const char* _name)
{
	view = new ApplicationView(_name, this);
	
}

Node_Application::~Node_Application()
{

}

bool Node_Application::init()
{
	// Create a context	
	this->ctx    = new Node_Container("Global");

	return view->init();;
}

void Node_Application::clear()
{
	this->ctx->clear();
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
	LOG_MSG("Node_Application::eval()");
	this->lastString 		= ctx->createNodeString(_expression.c_str());

	if ( lastString->getValueAsString() == "exit" ){
		stopExecution();		
	}else{
		if ( !lastString->isEmpty())
		{
			/* Create a Lexer node. The lexer will cut expression string into tokens
			(ex: "2*3" will be tokenized as : number"->"2", "operator"->"*", "number"->"3")*/
			auto lexer = ctx->createNodeLexer(lastString);
			return lexer->evaluate();
			//ctx->destroyNode(lexer);
		}
	}	

	return false;
}

void Node_Application::shutdown()
{
	LOG_MSG("Shutdown Nodable...\n");

	// Free memory
	delete this->exitString;
	delete this->ctx;
	delete this->lastString;
	delete this->view;
}

Node_Container* Node_Application::getContext()const
{
	return this->ctx;
}

