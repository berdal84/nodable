#pragma once

#include <nodable/core/types.h>
#include <nodable/core/Operator.h>
#include <nodable/core/InvokableFunction.h>

namespace Nodable {


	/**
		The role of this class is to link an operator with an implementation.
	*/
	class InvokableOperator: public IInvokable {
	public:



		InvokableOperator(const Operator* _operator, const IInvokable* _function  )
			: m_operator(_operator)
			, m_function(_function)
		{
			NODABLE_ASSERT(_operator)
			NODABLE_ASSERT(_function)
			NODABLE_ASSERT(static_cast<int>(_operator->type) == m_function->get_signature()->get_arg_count())
		}

		~InvokableOperator() override
        {
		    // delete m_function; no need to
		    // delete m_operator; owned by Language
        }

        inline const IInvokable*        get_function() const { return m_function; }
        inline const Operator*          get_operator() const { return m_operator; }
        inline Operator_t               get_operator_type() const { return m_operator->type; }
        inline std::string              get_short_identifier() const { return m_operator->identifier; }
        inline int                      get_precedence() const { return m_operator->precedence; }
        inline const FunctionSignature* get_signature() const override { return m_function->get_signature(); }
        inline IInvokable::Type         get_invokable_type() const override { return IInvokable::Type::OperatorFct; }
        inline void                     invoke(Member *_result, const std::vector<Member *> &_args) const override
        {
            m_function->invoke( _result, _args);
        }
    private:
        const Operator*   m_operator;
        const IInvokable* m_function;
    };
}
