#include <nodable/core/InvokableComponent.h>
#include <nodable/core/Log.h>
#include <nodable/core/Language.h>
#include <nodable/core/VariableNode.h>

using namespace Nodable;

REGISTER
{
    registration::push<InvokableComponent>("InvokableComponent");
}

InvokableComponent::InvokableComponent(const IInvokable* _invokable)
    : Component()
    , m_result( nullptr )
    , m_signature(_invokable->get_signature())
    , m_invokable(_invokable)
{
    m_args.resize(m_signature->get_arg_count(), nullptr );
    m_source_token = std::make_shared<Token>(Token_t::identifier, m_signature->get_label(), 0 );
}

InvokableComponent::InvokableComponent(const Signature* _signature)
    : Component()
    , m_result( nullptr )
    , m_signature(_signature)
    , m_invokable(nullptr)
{
    NODABLE_ASSERT(_signature != nullptr); // must be defined !
    m_args.resize(_signature->get_arg_count(), nullptr );
    m_source_token = std::make_shared<Token>(Token_t::identifier, _signature->get_label(), 0 );
}

bool InvokableComponent::update()
{
    bool success = true;

    auto not_declared_predicate = [](Member* _member)
    {
        auto var = _member->get_owner()->as<VariableNode>();
        return var && !var->is_declared();
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
            m_invokable->invoke(m_result, m_args);
        }
        catch (std::exception& err)
        {
            LOG_ERROR("InvokableComponent", "Exception thrown updating \"%s\" Component"
                                            " while updating Node \"%s\"."
                                            " Reason: %s\n",
                                            get_type().get_name(),
                                            get_owner()->get_label(),
                                            err.what() )
            success = false;
        }
    }

	return success;
}

