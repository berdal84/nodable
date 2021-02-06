#include <InstructionNode.h>
#include <Member.h>
#include <Log.h>

using namespace Nodable;

InstructionNode::InstructionNode(const char* _label): Node(_label)
{
    add("value", Visibility::Default, Type::Any, Way_In);
}

std::string InstructionNode::getTypeAsString()const
{
    return value()->getTypeAsString();
}