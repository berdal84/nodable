#include "InvokableComponent.h"

#include "fw/core/log.h"
#include "fw/core/reflection/func_type.h"

#include "core/VariableNode.h"

using namespace ndbl;

REGISTER
{
    fw::registration::push_class<InvokableComponent>("InvokableComponent").extends<Component>();
}

InvokableComponent::InvokableComponent(const fw::func_type* _signature, bool _is_operator, const fw::iinvokable* _invokable)
        : Component()
        , m_result( nullptr )
        , m_signature(_signature)
        , m_invokable(nullptr)
        , m_is_operator(_is_operator)
        , token(Token_t::identifier, _signature->get_identifier().c_str())
{
    FW_EXPECT(_signature != nullptr, "Signature must be defined!")
    m_invokable = _invokable;
    m_args.resize(_signature->get_arg_count(), nullptr );
}

bool InvokableComponent::update()
{
    bool success = true;

    auto not_declared_predicate = [](Property * _property)
    {
        auto var = fw::cast<const VariableNode>(_property->get_owner());
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
            // Get properties' variants, and invoke m_invokable with the variants as arguments
            std::vector<fw::variant*> args;
            Property::get_variant(m_args, args);
            auto result = m_invokable->invoke(args);
            m_result->get_variant()->set(result);

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
                                            get_owner()->name.c_str(),
                                            err.what()
                                            )
            success = false;
        }
    }

	return success;
}

