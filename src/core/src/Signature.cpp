#include <nodable/core/Signature.h>
#include <algorithm> // find_if
#include <nodable/core/constants.h>
#include <nodable/core/Operator.h>

using namespace Nodable;

Signature::Signature(std::string _id)
    : m_identifier(_id)
    , m_operator(nullptr)
    , m_return_type(R::get_meta_type<void>())
{
    clean_function_id(m_identifier);
}

Signature::Signature(const Operator* _operator)
    : Signature(_operator->identifier)
{
    NODABLE_ASSERT(_operator)
    m_identifier.insert(0, k_keyword_operator);
    m_operator = _operator;
}

void Signature::push_arg(R::MetaType_const_ptr _type)
{
   // create normalised name

   std::string name;

    if( m_operator)
    {
        switch ( m_args.size() + 1)
        {
            case 1: name = k_lh_value_member_name; break;
            case 2: name = k_rh_value_member_name; break;
            default: NODABLE_ASSERT(false)         break;
        }
    }
    else
    {
        name = k_func_arg_member_name_prefix + std::to_string(m_args.size());
    }

    m_args.emplace_back(_type, name);
}

bool Signature::is_exactly(const Signature* _other)const
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

bool Signature::is_compatible(const Signature* _other)const
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

bool Signature::has_an_arg_of_type(R::MetaType_const_ptr _type) const
{
    auto found = std::find_if( m_args.begin(), m_args.end(), [&_type](const FuncArg& each) { return each.m_type == _type; } );
    return found != m_args.end();
}

std::string Signature::get_label() const
{
    if( m_operator )
    {
        return m_operator->identifier;
    }
    return m_identifier;
}

std::string& Signature::clean_function_id(std::string& _id)
{
    return _id = regex_replace(_id, std::regex("^api_"), "");
}

const Signature* Signature::new_operator(
        Meta_t _type,
        const Operator* _op,
        Meta_t _ltype,
        Meta_t _rtype
)
{
    if(!_op)
    {
        return nullptr;
    }

    auto signature = new Signature(_op);
    signature->set_return_type(_type);
    signature->push_args(_ltype, _rtype);

    NODABLE_ASSERT(signature->is_operator())
    NODABLE_ASSERT(signature->get_arg_count() == 2)

    return signature;
}

const Signature* Signature::new_operator(
        Meta_t _type,
        const Operator* _op,
        Meta_t _ltype)
{
    if(!_op)
    {
        return nullptr;
    }

    auto signature = new Signature(_op);
    signature->set_return_type(_type);
    signature->push_arg(_ltype);

    NODABLE_ASSERT(signature->is_operator())
    NODABLE_ASSERT(signature->get_arg_count() == 1)

    return signature;
}