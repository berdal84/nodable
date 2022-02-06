#include <nodable/VariableNode.h>
#include <nodable/Member.h>
#include <nodable/Log.h>

using namespace Nodable;

REFLECT_DEFINE_CLASS(VariableNode)

VariableNode::VariableNode(Reflect::Type type)
    :
    Node("Variable"),
    m_typeToken(nullptr),
    m_identifierToken(nullptr),
    m_assignmentOperatorToken(nullptr)
{
	Member* value = m_props.add("value", Visibility::Always, type, Way_InOut);
}

bool VariableNode::eval() const
{
    if ( !isDefined() )
    {
        value()->define();
        Node::eval();
    }
    return true;
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

void VariableNode::define()
{
    value()->define();
}
