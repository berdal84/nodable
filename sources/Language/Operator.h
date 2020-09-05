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
	class Operator {
	public:
		Operator(
			std::string       _identifier,
			unsigned short    _precedence,
			FunctionSignature _prototype,
			FunctionImplem  _implementation):

			identifier(_identifier),
			precedence(_precedence),
			signature(_prototype),
			implementation(_implementation)
		{}
		~Operator() {}

		FunctionSignature signature;
		std::string       identifier;
		unsigned short    precedence;
		FunctionImplem  implementation;
	};
}
