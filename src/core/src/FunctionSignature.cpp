#include <nodable/FunctionSignature.h>
#include <algorithm> // find_if

using namespace Nodable;

FunctionSignature::FunctionSignature(std::string _identifier, std::string _label) :
        m_identifier(_identifier),
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
            if (m_args[i].m_type != _other->m_args[i].m_type && _other->m_args[i].m_type != Type_Any)
                is_matching = false;
            i++;
        }
    }
    return is_matching;
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

