#include "ASTNodeProperty.h"
#include "ndbl/core/language/Nodlang.h"

using namespace ndbl;
using namespace tools;

void ASTNodeProperty::init(
    ASTNode*                node,
    const TypeDescriptor*   type,
    PropertyFlags           flags,
    const char*             name
)
{
    VERIFY( node != nullptr     , "node is required!");
    VERIFY( m_type == nullptr   , "must be initialized once");
    VERIFY( type != nullptr     , "type can't be nullptr"   );

    m_node  = node;
    m_flags = flags;
    m_name  = name;

    set_type(type);
}

void ASTNodeProperty::digest(ASTNodeProperty* _property)
{
    m_token = std::move( _property->m_token );
}

bool ASTNodeProperty::is_type(const TypeDescriptor* other) const
{
    return m_type->equals( other );
}

void ASTNodeProperty::set_type(const tools::TypeDescriptor* new_type)
{
    if ( m_type == new_type ) return;

    m_type = new_type;

    // Make sure m_token matches with the new type if type changed
    
    //
    // TODO: In terms on responsiblities, it's not OK that this class
    //       initialize itself m_token because it requires access to the language.
    //       I think it would be more clear if we add an ASTNodeProperty factory function in ASTUtils
    //
    if (!has_language()) return; // Had to do this because when I test a node without a language, it crashes there.

    const Nodlang* language = get_language();
    // Convert m_type to a Token_t
    ASTToken_t token_type = language->to_literal_token(m_type);
    VERIFY(token_type != ASTToken_t::none, "This token is not handled");

    m_token = { token_type };
}
