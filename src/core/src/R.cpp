#include <nodable/R.h>
#include <type_traits> // std::underlying_type

using namespace Nodable::R;

std::shared_ptr<Type> Type::s_unknown = std::make_shared<Type>("unknown", "unknown", Typename::Null );

bool Type::has_qualifier(Qualifier _other_qualifier) const
{
    using T = std::underlying_type<Qualifier>::type;
    return
    (
        static_cast<T>(m_qualifier) & static_cast<T>(_other_qualifier)
    )
    !=
    static_cast <T>(Qualifier::Null);
}

void Type::add_qualifier(Qualifier _other_qualifier)
{
    using T = std::underlying_type_t<Qualifier>;
    m_qualifier = static_cast<Qualifier>( static_cast<T>(m_qualifier) | static_cast<T>(_other_qualifier) );
}

bool Type::is_ptr(const std::shared_ptr<const Type>& left)
{
    return  left->has_qualifier(Qualifier::Pointer);
}

bool Type::is_ref(const std::shared_ptr<const Type>& left)
{
    return left->has_qualifier(Qualifier::Ref);
}

std::shared_ptr<Type> Type::add_ref(std::shared_ptr<Type> left)
{
    left->add_qualifier(Qualifier::Ref);
    return left;
}

std::shared_ptr<Type> Type::add_ptr(std::shared_ptr<Type> left)
{
    left->add_qualifier(Qualifier::Pointer);
    return left;
}

bool Type::is_convertible(
        std::shared_ptr<const Type> _left,
        std::shared_ptr<const Type> _right )
{
    if(_left == Type::s_unknown || _right == Type::s_unknown ) // We allow cast to unknown type
    {
        return true;
    }
    else if (_left->get_typename() == _right->get_typename() )
    {
        return true;
    }
    else if (is_ptr(_left) && is_ptr(_right))
    {
        return true;
    }
    return false;
}

std::map<Typename, std::shared_ptr<const Type>>& TypeRegister::by_enum()
{
    static std::map<Typename, std::shared_ptr<const Type>> meta_register;
    return meta_register;
}

std::map<std::string, std::shared_ptr<const Type>>& TypeRegister::by_typeid()
{
    static std::map<std::string, std::shared_ptr<const Type>> type_register;
    return type_register;
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

bool TypeRegister::has_typeid(const std::string& _id)
{
    return by_typeid().find(_id) != by_typeid().end();
}

void Nodable::R::log_statistics()
{
    LOG_MESSAGE("R", "Logging reflected types ...\n");

    LOG_MESSAGE("R", "By typename (%i):\n", TypeRegister::by_enum().size() );
    for ( const auto& each : TypeRegister::by_enum() )
    {
        LOG_MESSAGE("R", " Typename::%s => %s \n", to_string(each.first), each.second->get_name() );
    }

    LOG_MESSAGE("R", "By typeid (%i):\n", TypeRegister::by_typeid().size() );
    for ( const auto& each : TypeRegister::by_typeid() )
    {
        LOG_MESSAGE("R", " %s => %s \n", each.first.c_str(), each.second->get_name() );
    }

    LOG_MESSAGE("R", "Logging done.\n");
}

void Nodable::R::init()
{
    TypeRegister::push<double>();
    TypeRegister::push<std::string>();
    TypeRegister::push<bool>();
    TypeRegister::push<void>();

    log_statistics();
}