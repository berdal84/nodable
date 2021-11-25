#include <nodable/InstructionNode.h>
#include <nodable/Member.h>
#include <nodable/Log.h>

using namespace Nodable;

REFLECT_DEFINE(InstructionNode)

InstructionNode::InstructionNode(const char* _label): Node(_label)
{
    m_props.add("value", Visibility::Default, Type_Any, Way_In);
    setNextMaxCount(1);
    setPrevMaxCount(-1);
}