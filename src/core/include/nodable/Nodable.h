#pragma once

#include <string> 	// for some typedefs
#include <map>	  	// for some typedefs
#include <vector> 	// for some typedefs
#include <assert.h> // for ASSERT and VERIFY
#include <nodable/Log.h>
#include <nodable/Variant.h>
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
    class Node;
    class NodeView;
    class VariableNode;

    enum class Relation_t: int
    {
        IS_CHILD_OF,
        IS_INPUT_OF,
        IS_SUCCESSOR_OF,
        IS_PREDECESSOR_OF,
        IS_OUTPUT_OF
    };

    struct Relation_link
    {
        Relation_link(Node* _src, Node* _dst): src(_src), dst(_dst){}
        Node* src;
        Node* dst;
    };

    enum ConnBy_
    {
        ConnectBy_Ref,
        ConnectBy_Copy
    };

    typedef long long i64_t;
    typedef long      i32_t;
    typedef int       i16_t;
    typedef short int i8_t;

    typedef std::map<std::string, Component*>  Components;
    typedef std::map<std::string, Member*>     Members;
    typedef std::vector<Wire*>                 Wires;
    typedef std::vector<VariableNode*> VariableNodes ;
    typedef std::vector<Node*>         Nodes ;
    typedef std::vector<NodeView*>     NodeViews ;
    typedef std::pair<const Relation_t, const Relation_link> Relation;
    typedef std::multimap<Relation::first_type, Relation::second_type> RelationRegistry;

    static bool operator==(const Relation_link& _left, const Relation_link& _right)
    {
        return (_left.src == _right.src) && (_left.dst == _right.dst);
    }

    static bool operator==(const Relation& _left, const Relation& _right)
    {
        return (_left.first == _right.first) && (_left.second == _right.second);
    }
}




