/*

Author: Bérenger Dalle-Cort, 2017

ChangeLog :

v0.3:
	- New Node_Assign : '=' can be used to assign a value to a symbol (ex: a = 10)
	- Now Able to perform binary operations on symbols (ex: c = a + b).
	- Node_Context : is now used as a factory.
	- Node : each node can get its contexts with Node::getParent()
	- Added a change log.
	- Added version number into the header file (NODABLE_VERSION_MAJOR, NODABLE_VERSION_MINOR, NODABLE_VERSION)

v0.2:
	- New Binary Operations : Node_Substract, Node_Multiply, Node_Divide
	- Node_Lexer : nos supports operator precedence.

v0.1:
	- Node_Add : to add two Node_Numbers
	- Node_Lexer : first version able to evaluate additions.
*/


/*
	Application version
*/

#pragma once
#define NODABLE_VERSION_MAJOR "0"
#define NODABLE_VERSION_MINOR "4"

#ifdef DEBUG
    #define NODABLE_VERSION NODABLE_VERSION_MAJOR "." NODABLE_VERSION_MINOR "(DEBUG)"
#else
    #define NODABLE_VERSION NODABLE_VERSION_MAJOR "." NODABLE_VERSION_MINOR "(RELEASE)"
#endif

/*
	Forward declarations
*/

namespace Nodable{	
	
	class Log;
	
	class Node;
	class Node_Value;
	class Node_Number;
	class Node_Add;
	class Node_Variable;
	class Node_Container;
	class Node_String;
	class Node_Lexer;
	class Node_BinaryOperation;
	class Node_Substract;
	class Node_Assign;
	class Node_Divide;
	class Node_Multiply;

	enum Type_{
		Type_Variable,
		Type_Number,
		Type_String,
		Type_COUNT
	};
}




