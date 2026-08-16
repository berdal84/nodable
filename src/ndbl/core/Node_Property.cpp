#include "Node_Property.h"
#include "ndbl/core/language/Nodlang.h"

void ndbl::property_init(
    Node_Property*          property,
    Node*                   node,
    const Type_Descriptor*  type,
    Node_Property::Flags    flags,
    const char*             name
)
{
    VERIFY( node != nullptr             , "node is required!");
    VERIFY( property->type == nullptr   , "must be initialized once");
    VERIFY( type != nullptr             , "type can't be nullptr"   );

    property->node  = node;
    property->flags = flags;
    property->name  = name;

    property_set_type(property, type);
}

void ndbl::property_digest(Node_Property* property, Node_Property* other)
{
    property->token = std::move( other->token );
}

void ndbl::property_set_type(Node_Property* property, const tools::Type_Descriptor* new_type)
{
    if ( property->type == new_type ) return;

    property->type = new_type;

    // Make sure m_token matches with the new type if type changed
    
    //
    // TODO: In terms on responsiblities, it's not OK that this class
    //       initialize itself m_token because it requires access to the language.
    //       I think it would be more clear if we add an Node_Property factory function in ASTUtils
    //
    if (!has_language()) return; // Had to do this because when I test a node without a language, it crashes there.

    const Nodlang* language = get_language();
    // Convert m_type to a Token_t
    Token_Type token_type = language->to_literal_token(property->type);
    VERIFY(token_type != Token_Type::none, "This token is not handled");

    property->token = { token_type };
}
