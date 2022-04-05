#include <nodable/core/FuncSig.h>
#include <algorithm> // find_if
#include <nodable/core/constants.h>

using namespace Nodable;

FuncSig::FuncSig(Type _type, std::string _identifier, std::string _label) :
        m_identifier(_identifier),
        m_type(_type),
        m_label(_label),
        m_return_type(R::get_meta_type<void>())
{

}

void FuncSig::push_arg(R::MetaType_const_ptr _type, std::string _name)
{
    if (_name.empty() )
    {
        if( m_type == FuncSig::Type::Function)
        {
            _name = "arg_" + std::to_string(m_args.size());
        }
        else
        {
            switch ( m_args.size() + 1)
            {
                case 1: _name = k_lh_value_member_name; break;
                case 2: _name = k_rh_value_member_name; break;
                default: NODABLE_ASSERT(false)          break;
            }
        }

    }
    m_args.emplace_back(_type, _name);
}

bool FuncSig::is_exactly(const FuncSig* _other)const
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

        if ( !arg_t->is_exactly( other_arg_t ) && arg_t->get_type() != other_arg_t->get_type() )
        {
            return false;
        }
        i++;
    }
    return true;
}

bool FuncSig::is_compatible(const FuncSig* _other)const
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
        else
        {
            return false;
        }
        i++;
    }
    return true;

}

bool FuncSig::has_an_arg_of_type(R::MetaType_const_ptr _type) const
{
    auto found = std::find_if( m_args.begin(), m_args.end(), [&_type](const FuncArg& each) { return each.m_type == _type; } );
    return found != m_args.end();
}

