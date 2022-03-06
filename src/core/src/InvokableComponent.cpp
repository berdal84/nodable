#include <nodable/InvokableComponent.h>
#include <nodable/Log.h>
#include <nodable/Language.h>
#include <nodable/VariableNode.h>

using namespace Nodable;

R_DEFINE_CLASS(InvokableComponent)

InvokableComponent::InvokableComponent(const IInvokable* _invokable)
    : Component()
    , m_result( nullptr )
    , m_invokable(_invokable)
{
    NODABLE_ASSERT(_invokable != nullptr); // must be defined !
    m_args.resize(_invokable->get_signature()->get_arg_count(), nullptr );
    m_source_token = std::make_shared<Token>(
            TokenType_Identifier,
            _invokable->get_signature()->get_label(),
            0 );
}

bool InvokableComponent::update()
{
    bool success = true;

    /** check if a member is an undeclared variable */
    auto found_member_not_declared = std::find_if(m_args.begin(), m_args.end(), [](Member* _member)
    {
        auto var = _member->get_owner()->as<VariableNode>();
        return var && !var->is_declared();
    });

    /** in case we found one, we do not invoke the invokable function */
    if (found_member_not_declared != m_args.end() )
    {
        success = false;
    }
    else
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
                                            get_class()->get_name(),
                                            get_owner()->get_label(),
                                            err.what() )
            success = false;
        }
    }


	return success;
}
