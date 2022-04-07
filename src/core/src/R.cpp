#include <nodable/core/reflection/R.h>
#include <type_traits> // std::underlying_type
#include <nodable/core/assertions.h>
#include "nodable/core/reflection/R_Meta_t.h"


using namespace Nodable::R;

Meta_t_csptr Meta_t::s_any = std::make_shared<Meta_t>("any", Type::any_t );

bool Meta_t::has_qualifier(Qualifier _other_qualifier) const
{
    using T = std::underlying_type<Qualifier>::type;
    return
    (
        static_cast<T>(m_qualifier) & static_cast<T>(_other_qualifier)
    )
    !=
    static_cast <T>(Qualifier::None);
}

void Meta_t::add_qualifier(Qualifier _other_qualifier)
{
    using T = std::underlying_type_t<Qualifier>;
    m_qualifier = static_cast<Qualifier>( static_cast<T>(m_qualifier) | static_cast<T>(_other_qualifier) );
}

bool Meta_t::is_ptr(Meta_t_csptr left)
{
    return  left->has_qualifier(Qualifier::Pointer);
}

bool Meta_t::is_ref(Meta_t_csptr left)
{
    return left->has_qualifier(Qualifier::Ref);
}

Meta_t_sptr Meta_t::add_ref(Meta_t_sptr left)
{
    left->add_qualifier(Qualifier::Ref);
    return left;
}

Meta_t_sptr Meta_t::add_ptr(Meta_t_sptr left)
{
    left->add_qualifier(Qualifier::Pointer);
    return left;
}

bool Meta_t::is_implicitly_convertible(Meta_t_csptr _left, Meta_t_csptr _right )
{
    if(_left == Meta_t::s_any || _right == Meta_t::s_any ) // We allow cast to unknown type
    {
        return true;
    }
    else if (_left->get_type() == _right->get_type() )
    {
        return true;
    }
    else if (is_ptr(_left) && is_ptr(_right))
    {
        return true;
    }

    switch( _left->get_type() )
    {
        case Type::i16_t:  return _right->get_type() == Type::double_t;
        default:           return false;
    }

}

bool Meta_t::is_exactly(Meta_t_csptr _other) const
{
    if( !_other) return false;

    return m_qualifier == _other->m_qualifier
           && m_type == _other->m_type;
}

Meta_t_csptr Meta_t::make_ptr(Meta_t_csptr _type)
{
    auto base_copy = std::make_shared<Meta_t>(*_type);
    return add_ptr(base_copy);
}

Meta_t_csptr Meta_t::make_ref(Meta_t_csptr _type)
{
    auto base_copy = std::make_shared<Meta_t>(*_type);
    return add_ref(base_copy);
}

std::string Meta_t::get_fullname() const
{
    std::string result;

    if (has_qualifier(Qualifier::Const))
    {
        result.append("const ");
    }

    result.append(m_name);

    if (has_qualifier(Qualifier::Pointer))
    {
        result.append("*");
    }
    else if (has_qualifier(Qualifier::Ref))
    {
        result.append("&");
    }

    return result;
}

bool Meta_t::is_ptr()const
{
    return has_qualifier(Qualifier::Pointer);
}

bool Meta_t::is_ref()const
{
    return has_qualifier(Qualifier::Ref);
}

std::map<Type, std::shared_ptr<const Meta_t>>& Register::by_type()
{
    static std::map<Type, std::shared_ptr<const Meta_t>> meta_type_register_by_category;
    return meta_type_register_by_category;
}

std::map<std::string, std::shared_ptr<const Meta_t>>& Register::by_typeid()
{
    static std::map<std::string, std::shared_ptr<const Meta_t>> meta_type_register_by_typeid;
    return meta_type_register_by_typeid;
}

bool Class::is_child_of(Class_ptr _possible_parent_class, bool _selfCheck) const
{
    bool is_child;

    if (_selfCheck && this == _possible_parent_class.get() )
    {
        is_child = true;
    }
    else if ( m_parents.empty())
    {
        is_child = false;
    }
    else
    {
        auto direct_parent_found = std::find(m_parents.begin(), m_parents.end(), _possible_parent_class);

        // direct parent check
        if ( direct_parent_found != m_parents.end())
        {
            is_child = true;
        }
        else // indirect parent check
        {
            bool is_a_parent_is_child_of = false;
            for (Class_ptr each : m_parents)
            {
                if (each->is_child_of(_possible_parent_class, true))
                {
                    is_a_parent_is_child_of = true;
                }
            }
            is_child = is_a_parent_is_child_of;
        }
    }
    return is_child;
};

void Class::add_parent(Class_ptr _parent)
{
    m_parents.insert(_parent);
}

void Class::add_child(Class_ptr _child)
{
    m_children.insert(_child);
}

bool Register::has_typeid(const std::string& _id)
{
    return by_typeid().find(_id) != by_typeid().end();
}

bool Nodable::R::Initialiser::s_initialized = false;

Nodable::R::Initialiser::Initialiser()
{
    if( s_initialized )
    {
        throw std::runtime_error("R has already been initialised !");
    }

    Register::push<double>();
    Register::push<std::string>();
    Register::push<bool>();
    Register::push<void>();
    Register::push<i16_t>();

    log_statistics();

    s_initialized = true;
}


void Nodable::R::Initialiser::log_statistics()
{
    LOG_MESSAGE("R", "Logging reflected types ...\n");

    LOG_MESSAGE("R", "By category (%i):\n", Register::by_type().size() );
    for ( const auto& each : Register::by_type() )
    {
        LOG_MESSAGE("R", " %s => %s \n", to_string(each.first), each.second->get_name() );
    }

    LOG_MESSAGE("R", "By typeid (%i):\n", Register::by_typeid().size() );
    for ( const auto& each : Register::by_typeid() )
    {
        LOG_MESSAGE("R", " %s => %s \n", each.first.c_str(), each.second->get_name() );
    }

    LOG_MESSAGE("R", "Logging done.\n");
}