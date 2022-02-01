
#pragma once
#include <string> 	// for some typedefs
#include <map>	  	// for some typedefs
#include <vector> 	// for some typedefs
#include <assert.h> // for ASSERT and VERIFY
#include <nodable/Log.h>

/*
	Asserts
*/
#define NODABLE_ASSERT(expression) LOG_FLUSH(); assert(expression);

#define NODABLE_ARRAYSIZE(_ARR)          ((int)(sizeof(_ARR)/sizeof(*_ARR))) // Size of a static C-style array. Don't use on pointers!

/*
	Forward declarations
*/

namespace Nodable
{
	class Wire;
	class Member;
	class Component;
    class VariableNode;

	typedef std::map<std::string, Component*>  Components;
	typedef std::map<std::string, Member*>     Members;
	typedef std::vector<Wire*>                 Wires;

    enum class Relation_t: int {
        IS_CHILD_OF,
        IS_INPUT_OF,
        IS_NEXT_OF,
        IS_OUTPUT_OF
    };

    enum ConnBy_ {
        ConnectBy_Ref,
        ConnectBy_Copy
    };

    typedef long long i64_t;
    typedef long      i32_t;
    typedef int       i16_t;
    typedef short int i8_t;

    typedef std::vector<VariableNode*> VariableNodes ;
}




