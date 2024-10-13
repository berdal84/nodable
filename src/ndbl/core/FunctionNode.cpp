#include "FunctionNode.h"

#include "tools/core/log.h"
#include "tools/core/memory/memory.h"
#include "tools/core/reflection/Type.h"

using namespace ndbl;
using namespace tools;

REFLECT_STATIC_INIT
{
    type::Initializer<FunctionNode>("FunctionNode").extends<Node>();
}

void FunctionNode::init(NodeType _type, const tools::FunctionDescriptor* _func_type )
{
    Node::init(_type, _func_type->get_identifier());

    m_func_type = _func_type;
    m_identifier_token = {
        Token_t::identifier,
        _func_type->get_identifier()
    };
    m_argument_slot.resize(_func_type->get_arg_count());
    m_argument_props.resize(_func_type->get_arg_count());

    add_slot( SlotFlag_PREV, Slot::MAX_CAPACITY );
    add_slot(SlotFlag_OUTPUT, 1);

    switch ( _type )
    {
        case NodeType_OPERATOR:
            set_name(_func_type->get_identifier());
            break;
        case NodeType_FUNCTION:
        {
            const std::string& id   = _func_type->get_identifier();
            std::string label       = id + "()";
            std::string short_label = id.substr(0, 2) + "..()"; // ------- improve, not great.
            set_name(label.c_str());
            break;
        }
        default:
            VERIFY(false, "Type not allowed")
    }

    // Create a result/value
    Property* value = add_prop(_func_type->get_return_type(), VALUE_PROPERTY );
    add_slot(SlotFlag_OUTPUT, Slot::MAX_CAPACITY, value);

    // Create arguments
    if ( _type == NodeType_OPERATOR )
    {
        VERIFY(_func_type->get_arg_count() >= 1, "An operator must have one argument minimum");
        VERIFY(_func_type->get_arg_count() <= 2, "An operator cannot have more than 2 arguments");
    }

    for (size_t i = 0; i < _func_type->get_arg_count(); i++ )
    {
        const FuncArg& arg  = _func_type->get_arg(i);

        const char* name;
        // TODO: this could be done in the NodeView instead...
        if ( _type == NodeType_OPERATOR )
        {
            if ( i == 0 )
                name = LEFT_VALUE_PROPERTY ;
            else if ( i == 1 )
                name = RIGHT_VALUE_PROPERTY;
        }
        else
        {
            name = arg.m_name.c_str();
        }

        Property* property  = add_prop(arg.m_type, name );

        if ( arg.m_by_reference )
            property->set_flags(PropertyFlag_IS_REF);

        m_argument_slot[i]  = add_slot(SlotFlag_INPUT, 1, property);
        m_argument_props[i] = property;
    }
}