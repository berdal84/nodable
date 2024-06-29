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

InvokableComponent::InvokableComponent(
    const func_type*  _signature,
    InvokableFlags    _flags,
    const IInvokable* _invokable
)
: NodeComponent()
, m_signature( _signature )
, token( Token_t::identifier, _signature->get_identifier().c_str() )
, m_flags(_flags)
, m_invokable(_invokable)
{
    EXPECT(_signature != nullptr, "Signature must be defined!")
    EXPECT((m_flags & InvokableFlag_WAS_INVOKED) == 0, "This flag is not allowed here")
    EXPECT((m_flags & InvokableFlag_IS_INVOKABLE) == 0, "This flag is not allowed here")

    if ( m_invokable != nullptr )
        m_flags |= InvokableFlag_IS_INVOKABLE;

    m_argument_slot.resize(_signature->get_arg_count());
}

void InvokableComponent::invoke()
{
    ASSERT(false) // THis code should be moved to the VirtualMachine (push args + exec native function on it)
//
//    ASSERT(m_flags & InvokableFlag_IS_INVOKABLE) // use has_flags(InvokableFlag_IS_INVOKABLE) before to call
//
//    // Prepare arguments to call the invokable with.
//    //
//    // Some properties do not reference anything, in such case we get their value as-is,
//    // but some properties are referencing a property (like when a variable is connected to an operator), in this case we get the referenced property's value.
//    //
//    std::vector<variant*> args;
//    args.reserve(m_argument_slot.size());
//    for(auto& slot: m_argument_slot )
//    {
//        Property* property = slot->get_property();
//        Slot*     adjacent = slot->first_adjacent();
//        if ( adjacent != nullptr && property->has_flags(PropertyFlag_IS_REF) )
//        {
//            property = adjacent->get_property();
//        }
//        args.emplace_back( property->value() );
//    }
//
//    // Call the invokable with the arguments
//    const variant result_value = m_invokable->invoke( args );
//
//    // Copy the result_value to the property's result slot
//    m_result_slot->get_property()->set( result_value );
//
//    // Flag as evaluated!
//    m_flags |= InvokableFlag_WAS_INVOKED;
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