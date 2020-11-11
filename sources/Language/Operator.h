#pragma once

#include <utility>
#include <memory>
#include "Function.h"

namespace Nodable {

	/**
		The role of this class is to link an operator string (ex: "=", ">=", "!=", etc.) with a Function.
		(cf. Function base class)

		Operators also have some additionnal informations to be parsed well: precedence !	

	Example: "+", "-" and "*" are an identifiers
			 0u, 1u and 2u are their respective precedence.
	*/
	class Operator: public Function {
	public:
		Operator(
			std::string       _identifier,
			unsigned short    _precedence,
            const std::shared_ptr<FunctionSignature>& _signature,
            FunctionImplem    _implementation):

			Function(_signature, std::move(_implementation)),
			identifier(std::move(_identifier)),
			precedence(_precedence)
		{}
		~Operator() = default;

		const std::string    identifier;
		const unsigned short precedence;
	};
}
