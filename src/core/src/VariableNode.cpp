#include <nodable/VariableNode.h>
#include <nodable/Member.h>
#include <nodable/Log.h>

using namespace Nodable;

REFLECT_DEFINE_CLASS(VariableNode)

VariableNode::VariableNode(Reflect::Type _type)
    :
        Node("Variable"),
        m_type_token(nullptr),
        m_identifier_token(nullptr),
        m_assignment_operator_token(nullptr)
{
	m_value = m_props.add(Node::VALUE_MEMBER_NAME, Visibility::Always, _type, Way_InOut);
}

bool VariableNode::eval() const
{
    if ( !is_defined() )
    {
        Node::eval();
    }
    return true;
}

void VariableNode::set_name(const char* _name)
{
    m_name = _name;
    std::string str = Reflect::to_string( m_value->get_type() );
    str.append(" ");
    str.append( _name );
    set_label(str);

	if (m_name.length() > 4)
    {
        set_short_label(m_name.substr(0, 3).append("..").c_str());
    }
    else
    {
        set_short_label(_name);
    }
}
