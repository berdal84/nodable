#pragma once
#include <memory>

#include "tools/core/types.h"
#include "tools/core/reflection/reflection"

#include "DirectedEdge.h"
#include "NodeComponent.h"
#include "Property.h"
#include "Token.h"

namespace ndbl
{
	/**
	  * @brief ComputeFunction extends Compute base to provide a Component that represents a Function.
	  *        This function has some arguments.
	  */
	class InvokableComponent : public NodeComponent
    {
	public:
        InvokableComponent();
		InvokableComponent(const tools::func_type*, bool _is_operator, const tools::IInvokable*);
		~InvokableComponent() = default;

        Token token;
        void                        invoke();
		void                        bind_arg(size_t _index, Slot*);
        const std::vector<Slot*>&   get_arguments() const;
		const tools::func_type*     get_func_type()const { return m_signature; }
		const tools::IInvokable*    get_function()const { return m_invokable; }
        void                        bind_result(Slot*);
        bool                        has_function() const { return m_invokable; };
		bool                        is_operator()const { return m_is_operator; };

    protected:
        Slot*                      m_result_slot;
        std::vector<Slot*>         m_argument_slot;
        const tools::func_type*    m_signature;
        const tools::IInvokable*   m_invokable;
        bool                       m_is_operator;

        REFLECT_DERIVED_CLASS()
    };
}


