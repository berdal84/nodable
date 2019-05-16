/*

Author: Bérenger Dalle-Cort, 2017

ChangeLog :

v0.3:
	- New Node_Assign : '=' can be used to assign a value to a symbol (ex: a = 10)
	- Now Able to perform binary operations on symbols (ex: c = a + b).
	- Node_Context : is now used as a factory.
	- Node : each node can get its contexts with Entity::getParent()
	- Added a change log.
	- Added version number into the header file (NODABLE_VERSION_MAJOR, NODABLE_VERSION_MINOR, NODABLE_VERSION)

v0.2:
	- New Binary Operations : Node_Substract, Node_Multiply, Node_Divide
	- Lexer : nos supports operator precedence.

v0.1:
	- Node_Add : to add two Node_Numbers
	- Lexer : first version able to evaluate additions.
*/

#pragma once
#include <string> 	// for some typedefs
#include <map>	  	// for some typedefs
#include <vector> 	// for some typedefs
#include <assert.h> // for ASSERT and VERIFY

/*
	Application Version
*/

#define NODABLE_VERSION_SHORT "0.4 wip"

#ifdef DEBUG
    #define NODABLE_VERSION NODABLE_VERSION_SHORT " (Debug) Build " __DATE__ " at " __TIME__
#else
    #define NODABLE_VERSION NODABLE_VERSION_SHORT " (Release) Build " __DATE__ " at " __TIME__
#endif

#define NODABLE_VERIFY(expression) assert(expression)

/*
	Asserts
*/

#ifdef DEBUG
	#define NODABLE_ASSERT(expression) assert(expression)
#else
    #define NODABLE_ASSERT(expression)
#endif


#define NODABLE_ARRAYSIZE(_ARR)          ((int)(sizeof(_ARR)/sizeof(*_ARR))) // Size of a static C-style array. Don't use on pointers!

/*
	Forward declarations
*/

namespace Nodable{ 

	// 0 - globals, check cpp for values.
    extern float bezierCurveOutRoundness;
    extern float bezierCurveInRoundness;
    extern float bezierThickness;
	extern float connectorRadius;
    extern bool  displayArrows; 

	// 1 - Common
	class Wire;
	class Member;
	class Variant;
	class Object;
	
	// 2 - Components :
	class Component;
	class History;

	// 2a - View Components
	class NodeView;
	class ContainerView;
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
	class Entity;
	class Application;	
	class Lexer;
	class Variable;
	class Container;

	// 4 - Other
	class Log;	

	typedef std::map<std::string, Component*>  Components;
	typedef std::map<std::string, Member*>      Members;
	typedef std::vector<Wire*>                 Wires;
}




