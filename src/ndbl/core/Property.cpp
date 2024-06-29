#include "Property.h"

#include "PropertyBag.h"
#include "VariableNode.h"
#include "tools/core/memory/memory.h"

using namespace ndbl;
using namespace tools;

void Property::init(const type* _type, PropertyFlags _flags, Node* _owner)
{
    EXPECT(m_type == nullptr, "must be initialized once")
    EXPECT(_type != nullptr, "type can't be nullptr")
    m_type  = _type;
    m_flags = _flags;
    m_owner = _owner;
}

void Property::digest(Property* _property)
{
    token = std::move( _property->token );
}

bool Property::is_type(const tools::type* other) const
{
    return m_type->equals( other );
}
