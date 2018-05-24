#include "Application.h"
#include "Nodable.h" 	// for NODABLE_VERSION
#include "Log.h" 		// for LOG_DBG
#include "Lexer.h"
#include "Node_Container.h"
#include "ApplicationView.h"
#include "Node_Variable.h"
#include <unistd.h>
#include <imgui.h>

using namespace Nodable;

Application::Application(const char* _name)
{
	LOG_MSG("A new Application ( label = \"%s\")\n", _name);
	setMember("class", "Application");
	setLabel(_name);
	addComponent("view", new ApplicationView(_name, this));
}

Application::~Application()
{

}

bool Application::init()
{
	LOG_MSG("init application ( label = \"%s\")\n", getLabel());
	this->ctx = std::unique_ptr<Node_Container>(new Node_Container("Global"));

	if( hasComponent("view"))
		return ((ApplicationView*)getComponent("view"))->init();

	return true;
}

void Application::clearContext()
{
	this->ctx.get()->clear();
}

bool Application::update()
{
	return !quit;
}

void Application::stopExecution()
{
	quit = true;
}

bool Application::eval(std::string _expression)
{
	LOG_MSG("Application::eval() - create a variable.\n");
	lastString = ctx->createNodeVariable("Command");

	LOG_DBG("Lexer::eval() - assign the expression string to that variable\n");
	lastString->setValue(_expression.c_str());

	LOG_DBG("Lexer::eval() - check if users type the exit keyword.\n");
	if ( lastString->getValueAsString() == "exit" ){
		LOG_DBG("Lexer::eval() - stopExecution...\n");
		stopExecution();		
	}else{
		LOG_DBG("Lexer::eval() - check if expression is not empty\n");
		if ( lastString->isSet())
		{
			/* Create a Lexer node. The lexer will cut expression string into tokens
			(ex: "2*3" will be tokenized as : number"->"2", "operator"->"*", "number"->"3")*/
			LOG_DBG("Lexer::eval() - create a lexer with the expression string\n");
			auto lexer = ctx->createNodeLexer(lastString);
			return lexer->eval();
			//ctx->destroyNode(lexer);
		}
	}	

	return false;
}

void Application::shutdown()
{
	LOG_MSG("shutting down application ( _name = \"%s\")\n", getLabel());
}

Node_Container* Application::getContext()const
{
	return this->ctx.get();
}

