#include "InvokableComponent.h"

#include "fw/core/log.h"
#include "fw/core/reflection/func_type.h"
#include "fw/core/Pool.h"

#include "core/VariableNode.h"

using namespace ndbl;
using fw::ID;

REGISTER
{
    fw::registration::push_class<InvokableComponent>("InvokableComponent").extends<Component>();
}

InvokableComponent::InvokableComponent()
    : Component()
    , m_signature( nullptr )
    , m_invokable( nullptr )
    , m_is_operator( false )
{}

InvokableComponent::InvokableComponent(const fw::func_type* _signature, bool _is_operator, const fw::iinvokable* _invokable)
    : Component()
    , m_signature( _signature )
    , m_invokable( nullptr)
    , m_is_operator( _is_operator )
    , token( Token_t::identifier, _signature->get_identifier().c_str() )
{
    FW_EXPECT(_signature != nullptr, "Signature must be defined!")
    m_invokable = _invokable;
    m_argument_slot.resize(_signature->get_arg_count());
}

bool InvokableComponent::update()
{
    if( !m_invokable )
    {
        return true;
    }

    try
    {
        // Gather the variants from argument slots
        std::vector<fw::variant*> variants;
        for(auto& slot: m_argument_slot )
        {
            variants.push_back( slot->first_adjacent()->get_property()->value() );
        }
        FW_ASSERT( m_argument_slot.size() == variants.size())
        fw::variant result = m_invokable->invoke( variants );
         m_result_slot->get_property()->set( result);
        for( auto* variant : variants ) variant->flag_defined(true);
    }
    catch (std::exception& err)
    {
        LOG_ERROR("InvokableComponent", "Exception thrown updating \"%s\" Component"
                                        " while updating Node \"%s\"."
                                        " Reason: %s\n",
                                        get_type()->get_name(),
                                        m_owner->name.c_str(),
                                        err.what()
                                        )
        return false;
    }
    return true;
}

void InvokableComponent::bind_result(SlotRef slot)
{
    FW_ASSERT(slot.flags & SlotFlag_OUTPUT);
    m_result_slot = slot;
}

void InvokableComponent::bind_arg(size_t arg_id, SlotRef slot)
{
    FW_ASSERT(slot.flags & SlotFlag_INPUT)
    m_argument_slot[arg_id] = slot;
}

const std::vector<SlotRef>& InvokableComponent::get_arguments() const
{
    return m_argument_slot;
}