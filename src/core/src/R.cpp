#include <nodable/R.h>
#include <nodable/Nodable.h>

using namespace Nodable::R;

Type* Type::s_unknown = new Type("unknown", "unknown", Typename::Null );

bool Type::has_qualifier(Qualifier _other_qualifier) const
{
    using T = std::underlying_type_t<Qualifier>;
    return
    (
        static_cast<T>(m_qualifier)
        &
        static_cast<T>(_other_qualifier)
    )
    !=
    static_cast <T>(Qualifier::Null);
}

void Type::add_qualifier(Qualifier _other_qualifier)
{
    using T = std::underlying_type_t<Qualifier>;
    m_qualifier = static_cast<Qualifier>( static_cast<T>(m_qualifier) | static_cast<T>(_other_qualifier) );
}

bool Type::is_ptr(const Type* left)
{
    NODABLE_ASSERT(left != nullptr);
    return  left->has_qualifier(Qualifier::Pointer);
}

bool Type::is_ref(const Type* left)
{
    NODABLE_ASSERT(left != nullptr);
    return left->has_qualifier(Qualifier::Ref);
}

Type* Type::add_ref(Type* left)
{
    left->add_qualifier(Qualifier::Ref);
    return left;
}

Type* Type::add_ptr(Type* left)
{
    left->add_qualifier(Qualifier::Pointer);
    return left;
}

bool Type::is_convertible( const Type* left, const Type* right )
{
    if( left == Type::s_unknown || right == Type::s_unknown ) // We allow cast to unknown type
    {
        return true;
    }
    return left->get_typename() == right->get_typename();
}

std::map<Typename, Type*>& Register::by_enum()
{
    static std::map<Typename, Type*> meta_register;
    return meta_register;
}

std::map<std::string, Type*>& Register::by_typeid()
{
    static std::map<std::string, Type*> type_register;
    return type_register;
}

bool Class::is_child_of(const Class *_possible_parent_class, bool _selfCheck) const
{
    bool is_child;

    if (_selfCheck && this == _possible_parent_class)
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
            for (Class *each : m_parents)
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

void Class::add_parent(Class *_parent)
{
    m_parents.insert(_parent);
}

void Class::add_child(Class *_child)
{
    m_children.insert(_child);
}
