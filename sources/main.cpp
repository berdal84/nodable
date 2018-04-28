#include <iostream>
#include <string>

#include "Nodable.h" // for NODABLE_VERSION
#include "Node.h"

using namespace std;
using namespace Nodable;

int main(int n, const char** args){


	cout << " -- Nodable " << NODABLE_VERSION << " - by Berdal84 - 2017 --" << endl;

	// Create a context
	auto ctx    = new Node_Context("Global");

	LOG_DBG("Launching the command line interpreter...");

	// Create few nodes to identify keywords :
	auto exit 				= ctx->createNodeString("exit"); 	/* define the Node_String "exit" as a keyword to exit application.*/
	auto lastString 		= ctx->createNodeString("");		/* Initialize a Node_String to store users input*/
	bool userWantsToQuit 	= false;	

	while( !userWantsToQuit )
	{
		/* Print command line prompt */
		printf(">>> ");

		/* Reads expression string written by user */
		char input[256];
		std::cin.getline(input,256);
		lastString = ctx->createNodeString(input);

		/* Create a Lexer node. The lexer will cut expression string into tokens
		(ex: "2*3" will be tokenized as : number"->"2", "operator"->"*", "number"->"3")*/
		auto lexer = ctx->createNodeLexer(lastString);
		lexer->evaluate();
		delete lexer;
		
		/* Checks if users wants to exit*/
		userWantsToQuit = (strcmp(lastString->getValue(), exit->getValue()) == 0);
	}

	return 0;
}

