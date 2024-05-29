#include "InvokableComponent.h"

#include "tools/core/log.h"
#include "tools/core/memory/Pool.h"
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

InvokableComponent::InvokableComponent(const func_type* _signature, bool _is_operator, const iinvokable* _invokable)
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

bool InvokableComponent::update()
{
    if( !m_invokable )
    {
        return true;
    }

    try
    {
        // Gather the variants from argument slots
        // TODO: consider removing this dynamic allocation
        //       we could simply dereference each Property within invoke()
        std::vector<variant*> variants;
        for(auto& slot: m_argument_slot )
        {
            Property* property = slot->get_property();
            Slot*     adjacent = slot->first_adjacent().get();
            if ( !property->is_ref() || adjacent == nullptr )
            {
                variants.push_back( property->value() );
            }
            else
            {
                variants.push_back( adjacent->get_property()->value() );
            }
        }
        ASSERT( m_argument_slot.size() == variants.size())
        variant result = m_invokable->invoke( variants );
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