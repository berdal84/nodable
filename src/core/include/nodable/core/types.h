#pragma once

#include <string>
#include <map>
#include <vector>

namespace ndbl
{
    /**
     * This file contains basic type declarations (and some forward declarations) we use everywhere in Nodable code.
     */

    // forward declarations
	class Edge;
	class Property;
	class Component;
    class Node;
    class NodeView;
    class VariableNode;

    // integers
    typedef long long i64_t;
    typedef long      i32_t;
    typedef int       i16_t;
    typedef short int i8_t;

    // unsigned integers
    typedef unsigned long long u64_t;
    typedef unsigned long      u32_t;
    typedef unsigned int       u16_t;
    typedef unsigned short int u8_t;

    // containers
    typedef std::map<std::string, Component*>       Components;
    typedef std::map<std::string, Property *>       PropertyMap;
    typedef std::vector<Property *>                 PropertyVec;
    typedef std::vector<Edge*> DirectedEdgeVec;
    typedef std::vector<VariableNode*>              VariableNodeVec;
    typedef std::vector<Node*>                      NodeVec ;
    typedef std::vector<NodeView*>                  NodeViewVec ;
}




