#include <VariableNode.h>
#include <Member.h>
#include <Log.h>

using namespace Nodable;

VariableNode::VariableNode()
    :
    Node("Variable"),
    m_typeToken(nullptr),
    m_identifierToken(nullptr),
    m_assignmentOperatorToken(nullptr)
{
	m_props.add("value", Visibility::Always, Type_Any, Way_InOut);
}

void VariableNode::setName(const char* _name)
{
    m_name = _name;
	setLabel(_name);

	if (m_name.length() > 4)
    {
        setShortLabel(m_name.substr(0, 3).append("..").c_str());
    }
    else
    {
        setShortLabel(_name);
    }

    value()->setSourceExpression(_name);
}