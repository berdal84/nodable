é#include <iostream>
#include <string>
#include "Nodable.h" 	// for NODABLE_VERSION
#include "Log.h" 		// for LOG_DBG
#include "Node.h"
#include "Node_String.h"
#include "Node_Lexer.h"
#include "Node_Container.h"
#include <unistd.h>

using namespace Nodable;

int main(int n, const char** args){

	LOG_MSG(" -- Nodable v%s - by Berdal84 - 2017 --\n", NODABLE_VERSION);

	// Create a context	
	auto ctx    = new Node_Container("Global");
	LOG_MSG("Launching the command line interpreter...\n");

	// Create few nodes to identify keywords :
	auto exitString 		= ctx->createNodeString("exit"); 	/* define the Node_String "exit" as a keyword to exit application.*/
	auto lastString 		= ctx->createNodeString("");		/* Initialize a Node_String to store users input*/	


	while( !lastString->isEqualsTo(exitString) )
	{
		if ( !lastString->isEmpty())
		{
			/* Create a Lexer node. The lexer will cut expression string into tokens
			(ex: "2*3" will be tokenized as : number"->"2", "operator"->"*", "number"->"3")*/
			auto lexer = ctx->createNodeLexer(lastString);
			lexer->evaluate();
			delete lexer;
		}

		/* Print command line prompt */
		LOG_MSG("\n>>> ");

		/* Reads expression string written by user */
		char input[256];
		std::cin.getline(input,256);
		delete lastString;
		lastString = ctx->createNodeString(input);
	}
	LOG_MSG("Shutdown Nodable...\n");

	// Free memory
	delete exitString;
	delete ctx;
	delete lastString;
	
	return 0;
}

