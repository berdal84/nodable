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
#include <string>
#include <map>
#include <vector>

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

    extern float bezierCurveOutRoundness;
    extern float bezierCurveInRoundness;
    extern float bezierThickness;
    extern bool displayArrows; 

	// 1 - Common
	class Wire;
	class Value;
	class Object;
	
	// 2 - Components :
	class Component;

	// 2a - View Components
	class NodeView;
	class ApplicationView;
	class WireView;
	
	// 2b - Binary Operation Components
	class BinaryOperationComponent;
	class Add;
	class Substract;
	class Assign;
	class Divide;
	class Multiply;

	// 3 - Nodes
	class Node;
	class Node_Application;	
	class Node_Lexer;
	class Node_Variable;
	class Node_Container;

	// 4 - Other
	class Log;	
	typedef std::map<std::string, Component*>  Components;
	typedef std::map<std::string, Value*>      Members;
	typedef std::vector<Wire*>                 Wires;
}




