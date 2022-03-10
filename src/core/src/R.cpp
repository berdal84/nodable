#include <nodable/R.h>
#include <type_traits> // std::underlying_type

using namespace Nodable::R;

std::shared_ptr<MetaType> MetaType::s_unknown = std::make_shared<MetaType>("unknown", Type::Null );

bool MetaType::has_qualifier(TypeQualifier _other_qualifier) const
{
    using T = std::underlying_type<TypeQualifier>::type;
    return
    (
        static_cast<T>(m_qualifier) & static_cast<T>(_other_qualifier)
    )
    !=
    static_cast <T>(TypeQualifier::None);
}

void MetaType::add_qualifier(TypeQualifier _other_qualifier)
{
    using T = std::underlying_type_t<TypeQualifier>;
    m_qualifier = static_cast<TypeQualifier>( static_cast<T>(m_qualifier) | static_cast<T>(_other_qualifier) );
}

bool MetaType::is_ptr(const std::shared_ptr<const MetaType>& left)
{
    return  left->has_qualifier(TypeQualifier::Pointer);
}

bool MetaType::is_ref(const std::shared_ptr<const MetaType>& left)
{
    return left->has_qualifier(TypeQualifier::Ref);
}

std::shared_ptr<MetaType> MetaType::add_ref(std::shared_ptr<MetaType> left)
{
    left->add_qualifier(TypeQualifier::Ref);
    return left;
}

std::shared_ptr<MetaType> MetaType::add_ptr(std::shared_ptr<MetaType> left)
{
    left->add_qualifier(TypeQualifier::Pointer);
    return left;
}

bool MetaType::is_convertible(
        std::shared_ptr<const MetaType> _left,
        std::shared_ptr<const MetaType> _right )
{
    if(_left == MetaType::s_unknown || _right == MetaType::s_unknown ) // We allow cast to unknown type
    {
        return true;
    }
    else if (_left->get_category() == _right->get_category() )
    {
        return true;
    }
    else if (is_ptr(_left) && is_ptr(_right))
    {
        return true;
    }
    return false;
}

bool MetaType::is(const std::shared_ptr<const MetaType> &_other) const
{
    return m_qualifier == _other->m_qualifier
           && m_category == _other->m_category;
}

std::shared_ptr<const MetaType> MetaType::make_ptr(const std::shared_ptr<const MetaType> &_type)
{
    auto base_copy = std::make_shared<MetaType>(*_type);
    return add_ptr(base_copy);
}

std::shared_ptr<const MetaType> MetaType::make_ref(const std::shared_ptr<const MetaType> &_type)
{
    auto base_copy = std::make_shared<MetaType>(*_type);
    return add_ref(base_copy);
}

std::map<Type, std::shared_ptr<const MetaType>>& Register::by_category()
{
    static std::map<Type, std::shared_ptr<const MetaType>> meta_type_register_by_category;
    return meta_type_register_by_category;
}

std::map<std::string, std::shared_ptr<const MetaType>>& Register::by_typeid()
{
    static std::map<std::string, std::shared_ptr<const MetaType>> meta_type_register_by_typeid;
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

void Nodable::R::log_statistics()
{
    LOG_MESSAGE("R", "Logging reflected types ...\n");

    LOG_MESSAGE("R", "By category (%i):\n", Register::by_category().size() );
    for ( const auto& each : Register::by_category() )
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

void Nodable::R::init()
{
    Register::push<double>();
    Register::push<std::string>();
    Register::push<bool>();
    Register::push<void>();

    log_statistics();
}