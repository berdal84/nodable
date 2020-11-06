
#pragma once
#include <string> 	// for some typedefs
#include <map>	  	// for some typedefs
#include <vector> 	// for some typedefs
#include <assert.h> // for ASSERT and VERIFY
#include <memory>   // for std::unique_ptr

#define NODABLE_VERIFY(expression) assert(expression)

/*
	Asserts
*/

#ifdef _DEBUG
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

	// Common
	class Wire;
	class Member;
	class Variant;
	class Object;
	
	// Components :
	class Component;
	class History;

	// View Components
	class NodeView;
	class ContainerView;
	class ApplicationView;
	class WireView;
	class FileView;

	// Binary Operation Components
	class ComputeBinaryOperation;
	class Add;
	class Subtract;
	class Assign;
	class Divide;
	class Multiply;

	// Nodes
	class Node;
	class Application;	
	class Parser;
	class Variable;
	class Container;

	// Other
	class Log;	
	class File;

	typedef std::map<std::string, std::shared_ptr<Component>>  Components;
	typedef std::map<std::string, Member*>     Members;
	typedef std::vector<Wire*>                 Wires;
}




