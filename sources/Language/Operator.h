#pragma once

#include "Nodable.h"
#include "Language.h"
#include "Function.h"

namespace Nodable {

	/**

	The Operator class store the identifier and the precedence of an operator, 
	he also has a CallableFunction in order to be evaluated.

	Example: "+", "-" and "*" are an identifiers
			 0u, 1u and 2u are their respective precedence.
	*/
	class Operator: public Function {
	public:
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
	};
}
