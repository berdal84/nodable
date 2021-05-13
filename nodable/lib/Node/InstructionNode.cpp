#include "InstructionNode.h"
#include "Core/Member.h"
#include "Core/Log.h"

using namespace Nodable;

InstructionNode::InstructionNode(const char* _label): Node(_label)
{
    m_props.add("value", Visibility::Default, Type_Any, Way_In);
    setNextMaxCount(1);
    setPrevMaxCount(-1);
}