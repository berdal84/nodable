#pragma once
#include <memory>

#include "tools/core/types.h"
#include "tools/core/reflection/reflection"

#include "DirectedEdge.h"
#include "Component.h"
#include "Property.h"
#include "Token.h"

namespace ndbl
{
	/**
	  * @brief ComputeFunction extends Compute base to provide a Component that represents a Function.
	  *        This function has some arguments.
	  */
	class InvokableComponent : public Component
    {
	public:
        InvokableComponent();
		InvokableComponent(const tools::func_type*, bool _is_operator, const tools::IInvokable*);
		~InvokableComponent() = default;
        InvokableComponent(InvokableComponent&&) = default;
        InvokableComponent& operator=(InvokableComponent&&) = default;

        Token token;
        void                        invoke();
		void                        bind_arg(size_t _index, SlotRef);
        const std::vector<SlotRef>& get_arguments() const;
		const tools::func_type*     get_func_type()const { return m_signature; }
		const tools::IInvokable*    get_function()const { return m_invokable; }
        void                        bind_result(SlotRef);
        bool                        has_function() const { return m_invokable; };
		bool                        is_operator()const { return m_is_operator; };

    protected:
        SlotRef                    m_result_slot;
        std::vector<SlotRef>       m_argument_slot;
        const tools::func_type*    m_signature;
        const tools::IInvokable*   m_invokable;
        bool                       m_is_operator;

        REFLECT_DERIVED_CLASS()
    };
}

static_assert(std::is_move_assignable_v<ndbl::InvokableComponent>, "Should be move assignable");
static_assert(std::is_move_constructible_v<ndbl::InvokableComponent>, "Should be move constructible");


