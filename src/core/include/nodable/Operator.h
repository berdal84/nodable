#pragma once

#include <nodable/Nodable.h>
#include <nodable/Language.h>
#include <nodable/Function.h>

namespace Nodable {

	/**
		The role of this class is to link an operator string (ex: "=", ">=", "!=", etc.) with a Function.
		(cf. Function base class)

		Operators also have some additionnal informations to be parsed well: precedence !	

	Example: "+", "-" and "*" are an identifiers
			 0u, 1u and 2u are their respective precedence.
	*/
	class Operator: public Invokable {
	public:

	    enum class Type: int
        {
	        Unary,
            Binary,
            Ternary
        };

		Operator( const Invokable* _function, int _precedence, const char* _short_identifier )
			:
			m_precedence(_precedence),
			m_short_identifier(_short_identifier),
			m_function(_function)
		{
            switch ( m_function->getSignature()->getArgCount() )
            {
                case 1: m_type = Type::Unary; break;
                case 2: m_type = Type::Binary; break;
                case 3: m_type = Type::Ternary; break;
            }
		}

        virtual void invoke(Member *_result, const std::vector<Member *> &_args) const override
        {
            m_function->invoke( _result, _args);
        }

        inline Type getType() const { return m_type; }

        inline std::string getShortIdentifier() const { return m_short_identifier; }

        inline int getPrecedence() const { return m_precedence; }

        inline const FunctionSignature* getSignature() const override { return m_function->getSignature(); }

    private:
        const Invokable* m_function;
		std::string m_short_identifier; // "+" not "operator+"
        int         m_precedence;
        Type        m_type;
    };
}
