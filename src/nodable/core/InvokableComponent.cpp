#include "InvokableComponent.h"

#include "fw/core/log.h"
#include "fw/core/reflection/func_type.h"
#include "fw/core/Pool.h"
#include "fw/core/algorithm.h"

#include "core/VariableNode.h"

using namespace ndbl;

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
    m_argument_id.resize(_signature->get_arg_count());
}

bool InvokableComponent::update()
{
    if( !m_invokable )
    {
        return true;
    }

    try
    {
        // Get properties' variants, and invoke m_invokable with the variants as arguments
        auto variants = fw::map<fw::variant*>(m_argument_id, [=](auto id) { return m_owner->get_prop_at(id)->value(); });
        fw::variant result = m_invokable->invoke( variants );
        m_owner->get_prop_at( m_result_id )->set( result);
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

void InvokableComponent::bind_result_property(size_t property_id)
{
    FW_EXPECT( m_owner->get_prop_at(property_id) != nullptr, "Property not found" );
    m_result_id = property_id;
}

Property::ID InvokableComponent::get_l_handed_val()
{
    return m_argument_id[0];
}

Property::ID InvokableComponent::get_r_handed_val()
{
    return m_argument_id[1];
}

Property::ID InvokableComponent::get_arg(size_t arg_id) const
{
    return m_argument_id[arg_id];
}

void InvokableComponent::bind_arg(size_t arg_id, size_t property_id)
{
    m_argument_id[arg_id] = property_id;
}

const std::vector<Property::ID>& InvokableComponent::get_arg_ids() const
{
    return m_argument_id;
}

std::vector<Property*> InvokableComponent::get_args() const
{
    std::vector<Property*> result;
    result.reserve(m_argument_id.size());
    auto& props = m_owner->props;
    std::transform(m_argument_id.begin(), m_argument_id.end(),
                   result.end(),
                   [&props](auto& each_id ) { return props.at(each_id); });
    return std::move(result);
}
