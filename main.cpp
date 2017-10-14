#include <iostream>
#include "Node.h"

using namespace std;
using namespace Nodable;

int main(int n, const char** args){

	cout << " -- Nodable v0.1 - by Berdal84 - 2017 --" << endl;

	// Create a context
	auto ctx    = new Node_Context("Global");

	// Create a simple graph that represent an addition named MyAddition
	{
		auto 		a 		= new Node_Integer(10);
		auto 		b 		= new Node_Integer(9);
		auto 		c 		= new Node_Integer();
		auto 		add 	= new Node_Add(a, b, c);
		Node_Tag* 	tag 	= new Node_Tag(ctx, "MyAddition", add);
	}

	// Try to find the node MyAddition and evaluate its value.
	auto node = ctx->find("MyAddition");
	if (node != nullptr)
		((Node_Add*)node->getValue())->evaluate();

	// Create few nodes to identify keywords :
	cout << endl;
	cout << "Launching the command line interpreter..." << endl;	

	auto exit 				= new Node_String("exit");
	auto lastString 		= new Node_String();
	bool userWantsToQuit 	= false;
	char input[256];

	while( !userWantsToQuit )
	{
		cout << ">>>";		
		cin >> input;
		lastString = new Node_String(input);
		cout << "result : " << lastString->getValue() << endl;
		userWantsToQuit = (strcmp(lastString->getValue(), exit->getValue()) == 0);
	}

	return 0;
}

