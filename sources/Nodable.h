
#pragma once
#include <string> 	// for some typedefs
#include <map>	  	// for some typedefs
#include <vector> 	// for some typedefs
#include <assert.h> // for ASSERT and VERIFY

/*
	Application Version
*/

#define NODABLE_VERSION_SHORT "0.5.0"

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
	extern float nodePadding;

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
	class FileView;

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
	class File;

	typedef std::map<std::string, Component*>  Components;
	typedef std::map<std::string, Member*>      Members;
	typedef std::vector<Wire*>                 Wires;
}




