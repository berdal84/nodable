#include <nodable/FunctionSignature.h>

using namespace Nodable;

FunctionSignature::FunctionSignature(std::string _identifier, Type _type, std::string _label) :
        m_identifier(_identifier),
        m_return_type(_type),
        m_label(_label)
{

}

void FunctionSignature::push_arg(Type _type, std::string _name) {
    if (_name == "" )
    {
        _name = "arg_" + std::to_string(m_args.size());
    }
    m_args.emplace_back(_type, _name);
}

bool FunctionSignature::match(const FunctionSignature* _other)const {

    if ( this == _other )
        return true;

    if ( m_args.size() != _other->m_args.size() )
        return false;

    if ( m_identifier != _other->m_identifier )
        return false;

    size_t i = 0;
    bool isMatching = true;
    while( i < m_args.size() && isMatching )
    {
        if (m_args[i].m_type != _other->m_args[i].m_type && _other->m_args[i].m_type != Type_Any)
            isMatching = false;
        i++;
    }

    return isMatching;
}

const std::string& FunctionSignature::get_identifier()const
{
    return this->m_identifier;
}

std::vector<FunctionArg> FunctionSignature::get_args() const
{
    return this->m_args;
}

Type FunctionSignature::get_return_type() const
{
    return m_return_type;
}

std::string FunctionSignature::get_label() const
{
    return m_label;
}

bool FunctionSignature::has_an_arg_of_type(Type _type) const
{
    auto found = std::find_if( m_args.begin(), m_args.end(), [&_type](const FunctionArg& each) { return  each.m_type == _type; } );
    return found != m_args.end();
}

