#pragma once

#include <string>
#include <map>
#include <vector>

namespace ndbl
{
    // forward declarations
	class Wire;
	class Member;
	class Component;
    class Node;
    class NodeView;
    class VariableNode;

    typedef long long i64_t;
    typedef long      i32_t;
    typedef int       i16_t;
    typedef short int i8_t;

    typedef unsigned long long u64_t;
    typedef unsigned long      u32_t;
    typedef unsigned int       u16_t;
    typedef unsigned short int u8_t;

    typedef std::map<std::string, Component*>       Components;
    typedef std::map<std::string, Member*>          MemberMap;
    typedef std::vector<Member*>                    MemberVec;
    typedef std::vector<Wire*>                      WireVec;
    typedef std::vector<VariableNode*>              VariableNodeVec;
    typedef std::vector<Node*>                      NodeVec ;
    typedef std::vector<NodeView*>                  NodeViewVec ;
}




