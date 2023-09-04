#pragma once
#include <memory>

#include "fw/core/types.h"
#include "fw/core/reflection/reflection"

#include "core/Edge.h"
#include "core/Component.h"
#include "core/Property.h"
#include "core/Token.h"

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
		InvokableComponent(const fw::func_type*, bool _is_operator, const fw::iinvokable*);
		~InvokableComponent() = default;
        InvokableComponent(InvokableComponent&&) = default;
        InvokableComponent& operator=(InvokableComponent&&) = default;

        Token token;
        bool                        update();
		void                        bind_arg(size_t arg_id, size_t property_id);
        Property::ID                get_arg(size_t _index)const;
		std::vector<Property*>      get_args()const;
        const std::vector<Property::ID>& get_arg_ids() const;
		const fw::func_type*        get_func_type()const { return m_signature; }
		const fw::iinvokable*       get_function()const { return m_invokable; }
        void                        bind_result_property(size_t property_id); // bind an existing property to the result of the computation
        Property::ID                get_l_handed_val();
        Property::ID                get_r_handed_val();
        bool                        has_function() const { return m_invokable; };
		bool                        is_operator()const { return m_is_operator; };

    protected:
        size_t                     m_result_id;
        std::vector<Property::ID>  m_argument_id;
        const fw::func_type*       m_signature;
        const fw::iinvokable*      m_invokable;
        bool                       m_is_operator;

        REFLECT_DERIVED_CLASS()
    };
}

static_assert(std::is_move_assignable_v<ndbl::InvokableComponent>, "Should be move assignable");
static_assert(std::is_move_constructible_v<ndbl::InvokableComponent>, "Should be move constructible");


