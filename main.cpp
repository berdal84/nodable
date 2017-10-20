#include <iostream>
#include <string>

#include "Node.h"

using namespace std;
using namespace Nodable;

int main(int n, const char** args){

	cout << " -- Nodable v0.1 - by Berdal84 - 2017 --" << endl;

	// Create a context
	auto ctx    = new Node_Context("Global");

	// Create a simple graph that represent an sumition named MySum
	{
		auto 		a 		= new Node_Number(10);
		auto 		b 		= new Node_Number(9);
		auto 		c 		= new Node_Number();
		auto 		sum 	= new Node_Add(a, b, c);
		Node_Tag* 	tag 	= new Node_Tag(ctx, "MySum", sum);
	}

	// Try to find the node MySum and evaluate its value.
	auto node = ctx->find("MySum");
	if (node != nullptr)
		((Node_Add*)node->getValue())->evaluate();

	// Create few nodes to identify keywords :
	cout << endl;
	cout << "Launching the command line interpreter..." << endl;	

	auto exit 				= new Node_String("exit"); 	/* define the Node_String "exit" as a keyword to exit application.*/
	auto lastString 		= new Node_String();		/* Initialize a Node_String to store users input*/
	bool userWantsToQuit 	= false;	

	while( !userWantsToQuit )
	{
		/* Print command line prompt */
		printf(">>> ");

		/* Reads expression string written by user */
		char input[256];
		std::cin.getline(input,256);
		lastString = new Node_String(input);

		/* Create a Lexer node. The lexer will cut expression string into tokens
		(ex: "2*3" will be tokenized as : number"->"2", "operator"->"*", "number"->"3")*/
		auto lexer = new Node_Lexer(lastString);
		lexer->evaluate();
		delete lexer;
		
		/* Checks if users wants to exit*/
		userWantsToQuit = (strcmp(lastString->getValue(), exit->getValue()) == 0);
	}

	return 0;
}

