#include <nodable/core/FunctionSignature.h>
#include <algorithm> // find_if

using namespace Nodable;

FunctionSignature::FunctionSignature(std::string _identifier, std::string _label) :
        m_identifier(_identifier),
        m_label(_label),
        m_return_type(R::get_meta_type<void>())
{

}

void FunctionSignature::push_arg(std::shared_ptr<const R::MetaType> _type, std::string _name)
{
    if (_name == "" )
    {
        _name = "arg_" + std::to_string(m_args.size());
    }
    m_args.emplace_back(_type, _name);
}

bool FunctionSignature::is_exactly(const FunctionSignature* _other)const
{
    if ( this == _other )                        return true;
    if ( m_args.size() != _other->m_args.size()) return false;
    if ( m_identifier != _other->m_identifier )  return false;
    if ( m_args.empty() )                        return true;

    size_t i = 0;
    while( i < m_args.size() )
    {
        auto &arg_t       = m_args[i].m_type;
        auto &other_arg_t = _other->m_args[i].m_type;

        if ( !arg_t->is_exactly( other_arg_t ) && arg_t->get_type() != other_arg_t->get_type())
        {
            return false;
        }
        i++;
    }
    return true;
}

bool FunctionSignature::is_compatible(const FunctionSignature* _other)const
{
    if ( this == _other )                        return true;
    if ( m_args.size() != _other->m_args.size()) return false;
    if ( m_identifier != _other->m_identifier )  return false;
    if ( m_args.empty() )                        return true;

    size_t i = 0;
    while( i < m_args.size() )
    {
        auto &arg_t       = m_args[i].m_type;
        auto &other_arg_t = _other->m_args[i].m_type;

        if ( arg_t->get_type() == other_arg_t->get_type() )
        {
        }
        else if ( arg_t->is_ref() && R::MetaType::is_implicitly_convertible(arg_t, other_arg_t))
        {
        }
        else if ( other_arg_t->get_type() == R::Type::null_t)
        {
        }
        else
        {
            return false;
        }
        i++;
    }
    return true;

}

bool FunctionSignature::has_an_arg_of_type(std::shared_ptr<const R::MetaType> _type) const
{
    auto found = std::find_if( m_args.begin(), m_args.end(), [&_type](const FunctionArg& each) { return  each.m_type == _type; } );
    return found != m_args.end();
}

