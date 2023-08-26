#include "InvokableComponent.h"

#include "fw/core/log.h"
#include "fw/core/reflection/func_type.h"
#include "fw/core/Pool.h"

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
    m_args.resize(_signature->get_arg_count());
}

bool InvokableComponent::update()
{
    bool success = true;

    auto not_declared_predicate = [](const Property* _property)
    {
        auto* variable_node = fw::cast<const VariableNode>( _property->owner().get() );
        return variable_node != nullptr  && !variable_node->is_declared();
    };

    /* avoid to invoke if arguments contains an undeclared variable */
    if ( std::find_if(m_args.begin(), m_args.end(), not_declared_predicate) != m_args.end() )
    {
        success = false;
    }
    else if( m_invokable )
    {
        try
        {
            // Get properties' variants, and invoke m_invokable with the variants as arguments
            auto result = m_invokable->invoke(Property::get(m_args) );
            m_result->set(result);

            for(auto arg : m_args)
            {
                arg->ensure_is_defined(true);
            }
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
            success = false;
        }
    }

	return success;
}

void InvokableComponent::bind_result_property(const char* property_name)
{
   m_result = m_owner->get_prop( property_name );
   FW_EXPECT( m_result != nullptr, "Property not found" );
}
