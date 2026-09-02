#include "Node_Property.h"
#include "core/Token_Type.h"
#include "ndbl/core/language/Nodlang.h"

namespace ndbl
{
    using namespace bdc;
    using namespace tools;

    void property_init(
        Node_Property*          property,
        Node*                   node,
        const Type_Descriptor*  type,
        Node_Property::Flags    flags,
        const bdc::String       name
    )
    {
        VERIFY( node    , "node is can't be nullptr");
        VERIFY( property, "property is can't be nullptr");
        VERIFY( type    , "type can't be nullptr"   );

        property->node  = node;
        property->flags = flags;
        property->name  = name;
        
        array_init(property->slots, 0);

        property_set_type(property, type);
    }

    void property_deinit( Node_Property* property)
    {
        array_release(property->slots);
        property->node  = nullptr;
        property->flags = 0;
        property->name  = "released";
    }

    void property_digest(Node_Property* property, Node_Property* other)
    {
        property->token = std::move( other->token );
    }

    void property_set_type(Node_Property* property, const tools::Type_Descriptor* new_type)
    {
        if ( property->type == new_type ) return;

        property->type = new_type;

        // Make sure m_token matches with the new type if type changed
        
        //
        // TODO: In terms on responsiblities, it's not OK that this class
        //       initialize itself m_token because it requires access to the language.
        //       I think it would be more clear if we add an Node_Property factory function in ASTUtils
        //
        if (!language_is_initialized()) return; // Had to do this because when I test a node without a language, it crashes there.

        // Convert m_type to a Token_t
        Token_Type token_type = lang_type_to_literal_token_type(language(), property->type);
        VERIFY(token_type != Token_Type_NULL, "This token is not handled");

        property->token = { token_type };
    }
} // namespace ndbl