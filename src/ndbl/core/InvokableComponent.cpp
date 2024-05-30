#include "InvokableComponent.h"

#include "tools/core/log.h"
#include "tools/core/memory/memory.h"
#include "tools/core/reflection/func_type.h"

#include "VariableNode.h"

using namespace ndbl;
using namespace tools;

REFLECT_STATIC_INIT
{
    StaticInitializer<InvokableComponent>("InvokableComponent").extends<Component>();
}

InvokableComponent::InvokableComponent()
    : Component()
    , m_signature( nullptr )
    , m_invokable( nullptr )
    , m_is_operator( false )
{}

InvokableComponent::InvokableComponent(const func_type* _signature, bool _is_operator, const IInvokable* _invokable)
    : Component()
    , m_signature( _signature )
    , m_invokable( nullptr)
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
        Slot*     adjacent = slot->first_adjacent().get();
        if ( adjacent != nullptr && property->is_ref() )
        {
            property = adjacent->get_property();
        }
        args.emplace_back( property->value() );
    }

    // 2) call the invokable with the arguments
    variant result_value = m_invokable->invoke( args );

    // 3) copy the result_value to the property's result slot
    m_result_slot->get_property()->set( result_value );

    // WIP: we temporarily set all the arguments to "defined", this is a hack to deal with lvalue references.
    //      Ex:  calling T& operator=(T& lvalue, const T& rvalue) might modify lvalue.
    //
    // TODO: flag_defined() only when argument is non-const
    //
    for( auto* variant : args )
    {
        variant->flag_defined(true);
    }
}

void InvokableComponent::bind_result(SlotRef slot)
{
    ASSERT(slot.flags & SlotFlag_OUTPUT);
    m_result_slot = slot;
}

void InvokableComponent::bind_arg(size_t arg_id, SlotRef slot)
{
    ASSERT(slot.flags & SlotFlag_INPUT)
    m_argument_slot[arg_id] = slot;
}

const std::vector<SlotRef>& InvokableComponent::get_arguments() const
{
    return m_argument_slot;
}