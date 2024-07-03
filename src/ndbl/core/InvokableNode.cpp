#include "InvokableNode.h"

#include "tools/core/log.h"
#include "tools/core/memory/memory.h"
#include "tools/core/reflection/FuncType.h"

#include "VariableNode.h"

using namespace ndbl;
using namespace tools;

REFLECT_STATIC_INIT
{
    StaticInitializer<InvokableNode>("InvokableNode").extends<Node>();
}

void InvokableNode::init(NodeType _type, const FuncType*  _func_type )
{
    Node::init(_type, _func_type->get_identifier());

    VERIFY(_func_type != nullptr, "Signature must be defined!")
    m_func_type = _func_type;
    m_identifier_token = {
        Token_t::identifier,
        _func_type->get_identifier().c_str()
    };
    m_argument_slot.resize(_func_type->get_arg_count());
    m_argument_props.resize(_func_type->get_arg_count());

    add_slot( SlotFlag_PREV, Slot::MAX_CAPACITY );
    add_slot(SlotFlag_OUTPUT, 1);

    switch ( _type )
    {
        case NodeType_OPERATOR:
            set_name(_func_type->get_identifier().c_str());
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
        size_t count = _func_type->get_arg_count();
        VERIFY(count != 0, "An operator cannot have zero argument");
        VERIFY(count < 3, "An operator cannot have more than 2 arguments");
    }

    for (size_t i = 0; i < get_arg_slots().size(); i++ )
    {
        const FuncArg& arg  = _func_type->get_args()[i];
        Property* property  = add_prop(arg.m_type, arg.m_name.c_str(), arg.m_by_reference * PropertyFlag_IS_REF );
        m_argument_slot[i]  = add_slot(SlotFlag_INPUT, 1, property);
        m_argument_props[i] = property;

        // TODO: this should be done in the NodeView instead...
        if ( _type == NodeType_OPERATOR )
        {
            if ( i == 0 )
                property->set_name( LEFT_VALUE_PROPERTY );
            else if ( i == 1 )
                property->set_name( RIGHT_VALUE_PROPERTY );
        }
    }
}

const std::vector<Slot*>& InvokableNode::get_arg_slots() const
{
    return m_argument_slot;
}