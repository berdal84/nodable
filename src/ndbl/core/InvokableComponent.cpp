#include "InvokableComponent.h"

#include "tools/core/log.h"
#include "tools/core/memory/memory.h"
#include "tools/core/reflection/func_type.h"

#include "VariableNode.h"

using namespace ndbl;
using namespace tools;

REFLECT_STATIC_INIT
{
    StaticInitializer<InvokableComponent>("InvokableComponent").extends<NodeComponent>();
}

InvokableComponent::InvokableComponent()
    : NodeComponent()
{}

InvokableComponent::InvokableComponent(const func_type* _signature, bool _is_operator, const IInvokable* _invokable)
    : NodeComponent()
    , m_signature( _signature )
    , m_is_operator( _is_operator )
    , token( Token_t::identifier, _signature->get_identifier().c_str() )
{
    EXPECT(_signature != nullptr, "Signature must be defined!")
    m_invokable = _invokable;
    m_argument_slot.resize(_signature->get_arg_count());
}

void InvokableComponent::invoke()
{
    if( !m_invokable )
    {
        return;
    }

    // 1) prepare arguments to call the invokable with.
    //
    // Some properties do not reference anything, in such case we get their value as-is,
    // but some properties are referencing a property (like when a variable is connected to an operator), in this case we get the referenced property's value.
    //
    std::vector<variant*> args;
    args.reserve(m_argument_slot.size());
    for(auto& slot: m_argument_slot )
    {
        Property* property = slot->get_property();
        Slot*     adjacent = slot->first_adjacent();
        if ( adjacent != nullptr && property->is_ref() )
        {
            property = adjacent->get_property();
        }
        args.emplace_back( property->value() );
    }

    // 2) call the invokable with the arguments
    const variant result_value = m_invokable->invoke( args );

    // 3) copy the result_value to the property's result slot
    m_result_slot->get_property()->set( result_value );

    // Hack:
    //  Here we manually set to "defined" each of the argument variants.
    //  We have to do it because we are not assigning the variant's data ourselves, it happens within
    //  the m_invokable->invoke( args ) call which lead to a native CPP function call (where variant concepts are
    //  not present.
    //  We know of course that the invoke call will imply to assign a new value to result_value variant.
    //  But, we also have to deal with more complicated cases like references. In such case, the variant can be mutated
    //  from the outside.
    //  By consequences, I choose to set the defined flag for all the arguments.
    for(variant* each : args )
        each->flag_defined();
}

void InvokableComponent::bind_result(Slot* slot)
{
    ASSERT(slot != nullptr)
    ASSERT(slot->has_flags(SlotFlag_OUTPUT) );
    m_result_slot = slot;
}

void InvokableComponent::bind_arg(size_t arg_id, Slot* slot)
{
    ASSERT(slot != nullptr)
    ASSERT(slot->has_flags(SlotFlag_INPUT))
    m_argument_slot[arg_id] = slot;
}

const std::vector<Slot*>& InvokableComponent::get_arguments() const
{
    return m_argument_slot;
}