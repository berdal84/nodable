#include <nodable/core/VariableNode.h>
#include <nodable/core/Member.h>
#include <nodable/core/Log.h>

using namespace Nodable;

R_DEFINE_CLASS(VariableNode)

VariableNode::VariableNode(std::shared_ptr<const R::MetaType> _type)
    : Node("Variable")
    , m_type_token(nullptr)
    , m_identifier_token(nullptr)
    , m_assignment_operator_token(nullptr)
    , m_is_declared(true)
    , m_scope(nullptr)
{
	m_value = m_props.add(k_value_member_name, Visibility::Always, _type, Way_InOut);
}

void VariableNode::set_name(const char* _name)
{
    m_name = _name;
    std::string str;

    // append type only if not unknown
    if (m_value->get_meta_type())
    {
        str.append(m_value->get_meta_type()->get_name() );
        str.append(" ");
    }

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

bool VariableNode::eval() const
{
    if( never_assigned() && !m_value->is_initialized() )
    {
        m_value->set_initialized(true);
        return Node::eval();
    }
    return true;
}

bool VariableNode::never_assigned() const
{
    return !m_value->get_data()->is_defined();
}
