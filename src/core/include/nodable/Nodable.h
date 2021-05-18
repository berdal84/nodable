
#pragma once
#include <string> 	// for some typedefs
#include <map>	  	// for some typedefs
#include <vector> 	// for some typedefs
#include <assert.h> // for ASSERT and VERIFY
#include <nodable/Log.h>

#define NODABLE_VERIFY(expression) assert(expression)

/*
	Asserts
*/

#ifdef _DEBUG
	#define NODABLE_ASSERT(expression) LOG_FLUSH(); assert(expression)
#else
    #define NODABLE_ASSERT(expression)
#endif


#define NODABLE_ARRAYSIZE(_ARR)          ((int)(sizeof(_ARR)/sizeof(*_ARR))) // Size of a static C-style array. Don't use on pointers!

/*
	Forward declarations
*/

namespace Nodable
{
	class Wire;
	class Member;
	class Component;

	typedef std::map<std::string, Component*>  Components;
	typedef std::map<std::string, Member*>     Members;
	typedef std::vector<Wire*>                 Wires;

    enum class RelationType: int {
        IS_CHILD_OF,
        IS_INPUT_OF,
        IS_NEXT_OF,
        IS_OUTPUT_OF
    };
}




