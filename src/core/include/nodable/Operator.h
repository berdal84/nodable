#pragma once

#include <nodable/Nodable.h>
#include <nodable/Language.h>
#include <nodable/Function.h>

namespace Nodable::core {

	/**
		The role of this class is to link an operator string (ex: "=", ">=", "!=", etc.) with a Function.
		(cf. Function base class)

		Operators also have some additionnal informations to be parsed well: precedence !	

	Example: "+", "-" and "*" are an identifiers
			 0u, 1u and 2u are their respective precedence.
	*/
	class Operator: public Function {
	public:

	    enum class Type: int
        {
	        Unary,
            Binary,
            Ternary
        };

		Operator(
			std::string       _identifier,
			unsigned short    _precedence,
			FunctionSignature _signature,
			FunctionImplem    _implementation):

			Function(_signature, _implementation),
			identifier(_identifier),
			precedence(_precedence)
		{}
		~Operator() {}

		const std::string    identifier;
		const unsigned short precedence;

        Operator::Type getType() const
        {
            switch (signature.getArgs().size()) {
                case 1: return Type::Unary;
                case 2: return Type::Binary;
                case 3: return Type::Ternary;
                default: assert(false);
            }
        }
    };
}
