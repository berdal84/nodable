#include <nodable/core/InvokableComponent.h>
#include <nodable/core/Log.h>
#include <nodable/core/Language.h>
#include <nodable/core/VariableNode.h>

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

    auto not_declared_predicate = [](Member* _member)
    {
        auto var = _member->get_owner()->as<VariableNode>();
        return var && !var->is_declared();
    };

    //auto not_defined_predicate = [](Member* _member)
    //{
    //    return _member && !_member->is_defined();
    //};

    /* avoid to invoke if arguments contains an undeclared variable */
    if ( std::find_if(m_args.begin(), m_args.end(), not_declared_predicate) != m_args.end() )
    {
        success = false;
    }
    /* avoid to invoke if arguments contains an undefined */
    //else if ( std::find_if(m_args.begin(), m_args.end(), not_defined_predicate ) != m_args.end() )
    //{
    //    success = false;
    //}
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
