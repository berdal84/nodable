#include <nodable/VariableNode.h>
#include <nodable/Member.h>
#include <nodable/Log.h>

using namespace Nodable;

REFLECT_CLASS_DEFINITION(VariableNode)

VariableNode::VariableNode(Type type)
    :
    Node("Variable"),
    m_typeToken(nullptr),
    m_identifierToken(nullptr),
    m_assignmentOperatorToken(nullptr)
{
	m_props.add("value", Visibility::Always, type, Way_InOut);
}

void VariableNode::setName(const char* _name)
{
    m_name = _name;
    std::string str = getTypeAsString();
    str.append(" ");
    str.append( _name );
	setLabel(str);

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