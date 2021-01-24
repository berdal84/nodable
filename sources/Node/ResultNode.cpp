#include <ResultNode.h>
#include <Member.h>
#include <Log.h>

using namespace Nodable;

ResultNode::ResultNode(const char* _label): Node(_label)
{
    add("value", Visibility::Default, Type::Any, Way_In);
}

std::string ResultNode::getTypeAsString()const
{
    return value()->getTypeAsString();
}