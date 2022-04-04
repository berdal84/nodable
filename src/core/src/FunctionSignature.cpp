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
    bool is_exactly;

    if ( this == _other )
    {
        is_exactly = true;
    }
    else if ( m_args.size() != _other->m_args.size() || m_identifier != _other->m_identifier )
    {
        is_exactly = false;
    }
    else
    {
        size_t i = 0;
        is_exactly = true;
        while( i < m_args.size() && is_exactly )
        {
            if ( !m_args[i].m_type->is_exactly( _other->m_args[i].m_type ) )
                is_exactly = false;
            i++;
        }
    }
    return is_exactly;
}

bool FunctionSignature::is_compatible(const FunctionSignature* _other)const
{
    bool is_matching;

    if ( this == _other )
    {
        is_matching = true;
    }
    else if ( m_args.size() != _other->m_args.size() || m_identifier != _other->m_identifier )
    {
        is_matching = false;
    }
    else
    {
        size_t i = 0;
        is_matching = true;
        while( i < m_args.size() && is_matching )
        {
            auto &arg_t       = m_args[i].m_type;
            auto &other_arg_t = _other->m_args[i].m_type;

            if ( arg_t->get_type() != other_arg_t->get_type() || arg_t->is_ref() || !R::MetaType::is_implicitly_convertible(arg_t, other_arg_t))
            {
                is_matching = false;
            }
            i++;
        }
    }
    return is_matching;
}

bool FunctionSignature::has_an_arg_of_type(std::shared_ptr<const R::MetaType> _type) const
{
    auto found = std::find_if( m_args.begin(), m_args.end(), [&_type](const FunctionArg& each) { return  each.m_type == _type; } );
    return found != m_args.end();
}

