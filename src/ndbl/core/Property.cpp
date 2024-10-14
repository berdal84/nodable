#include "Property.h"

#include "PropertyBag.h"
#include "VariableNode.h"
#include "tools/core/memory/memory.h"

using namespace ndbl;
using namespace tools;

void Property::init(const TypeDescriptor* _type, PropertyFlags _flags, Node* _owner, const char* _name)
{
    VERIFY(m_type == nullptr, "must be initialized once")
    VERIFY(_type != nullptr, "type can't be nullptr")
    m_type  = _type;
    m_flags = _flags;
    m_owner = _owner;
    m_name  = _name;
}

void Property::digest(Property* _property)
{
    m_token = std::move( _property->m_token );
}

bool Property::is_type(const TypeDescriptor* other) const
{
    return m_type->equals( other );
}
