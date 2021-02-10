#include <InstructionNode.h>
#include <Member.h>
#include <Log.h>

using namespace Nodable;

InstructionNode::InstructionNode(
        const char* _label,
        CodeBlockNode* _parent): Node(_label)
{
    add("value", Visibility::Default, Type::Any, Way_In);
    this->parent = _parent;
}

std::string InstructionNode::getTypeAsString()const
{
    return value()->getTypeAsString();
}