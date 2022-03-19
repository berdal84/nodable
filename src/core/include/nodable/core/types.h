#pragma once

#include <string>
#include <map>
#include <vector>
#include <nodable/core/Log.h>
#include <nodable/core/Variant.h>
#include <nodable/core/reflection/R.h>
#include <nodable/core/Edge.h>

namespace Nodable
{
    // forward declarations
	class Wire;
	class Member;
	class Component;
    class Node;
    class NodeView;
    class VariableNode;

    typedef long long i64;
    typedef long      i32;
    typedef int       i16;
    typedef short int i8;

    typedef unsigned long long u64;
    typedef unsigned long      u32;
    typedef unsigned int       u16;
    typedef unsigned short int u8;

    typedef std::map<std::string, Component*>       Components;
    typedef std::map<std::string, Member*>          Members;
    typedef std::vector<Wire*>                      Wires;
    typedef std::vector<VariableNode*>              VariableNodes;
    typedef std::vector<Node*>                      Nodes ;
    typedef std::vector<NodeView*>                   NodeViews ;
    typedef std::multimap<EdgeType, const DirectedEdge> RelationRegistry;
}




