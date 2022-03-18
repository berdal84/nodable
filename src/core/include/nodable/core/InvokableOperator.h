#pragma once

#include <nodable/core/types.h>
#include <nodable/core/Language.h>
#include <nodable/core/InvokableFunction.h>

namespace Nodable {

	/**
		The role of this class is to link an operator string (ex: "=", ">=", "!=", etc.) with a Function.
		(cf. Function base class)

		Operators also have some additionnal informations to be parsed well: precedence !	

	Example: "+", "-" and "*" are an identifiers
			 0u, 1u and 2u are their respective precedence.
	*/
	class InvokableOperator: public IInvokable {
	public:

	    enum class Type: int
        {
	        Unary,
            Binary,
            Ternary
        };

		InvokableOperator(const IInvokable* _function, int _precedence, const char* _short_identifier )
			:
			m_precedence(_precedence),
			m_short_identifier(_short_identifier),
			m_function(_function)
		{
            switch (m_function->get_signature()->get_arg_count() )
            {
                case 1: m_type = Type::Unary; break;
                case 2: m_type = Type::Binary; break;
                case 3: m_type = Type::Ternary; break;
            }
		}

        inline Type                     get_operator_type() const { return m_type; }
        inline std::string              get_short_identifier() const { return m_short_identifier; }
        inline int                      get_precedence() const { return m_precedence; }
        inline const FunctionSignature* get_signature() const override { return m_function->get_signature(); }
        inline IInvokable::Type          get_invokable_type() const override { return IInvokable::Type::Operator; }
        inline void                     invoke(Member *_result, const std::vector<Member *> &_args) const override
        {
            m_function->invoke( _result, _args);
        }
    private:
        const IInvokable* m_function;
		std::string m_short_identifier; // like "+", not like "operator+"
        int         m_precedence;
        Type        m_type;
    };
}
