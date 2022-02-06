#include <nodable/InvokableComponent.h>
#include <nodable/Log.h>
#include <nodable/Language.h>

using namespace Nodable;

REFLECT_DEFINE_CLASS(InvokableComponent)

InvokableComponent::InvokableComponent(const Invokable* _invokable)
    : Component()
    , m_result( nullptr )
    , m_source_token( Token::s_null )
    , m_invokable(_invokable)
{
    NODABLE_ASSERT(_invokable != nullptr); // must be defined !
    m_args.resize(_invokable->get_signature()->get_arg_count(), nullptr );

}

bool InvokableComponent::update()
{
    bool success = true;

    try
    {
        m_invokable->invoke(m_result, m_args);
    }
    catch (std::runtime_error& err)
    {
        LOG_ERROR("InvokableComponent", "Exception thrown during update %s:\n"
                                        " - while updating Node %s\n"
                                        " - reason: %s\n", get_owner()->getLabel(), err.what() )
        success = false;
    }

	return success;
}
