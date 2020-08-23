#pragma once

#include "Nodable.h"
#include "LanguageEnums.h"
#include <functional>

namespace Nodable {

	/*
	 * Type of a callable function.
	 * Require an integer as output (works like an error code, 0: OK, 1 >= : error)
	 * An output member and some arguments
     *
	 * TODO: try to replace Member* by Variant*
	 */
	typedef std::function <int (Member*, const std::vector<const Member*>&)> FunctionImplem;

	/*
	 * Simple object to store a function argument (token, name)
	 */
	class FunctionArg {
	public:
		FunctionArg(TokenType_, std::string);
		TokenType_ type;
		std::string name;
	};

	/*
	 * Class to store a function signature.
	 * We can check if two function signature are matching using this->match(other)
	 */
	class FunctionSignature {
	public:
		FunctionSignature(std::string _identifier, TokenType_ _type, std::string _label = "");
		~FunctionSignature() {};
		void                           pushArg(TokenType_ _type, std::string _name = "");
		bool                           match(FunctionSignature& _other);
		const std::string&             getIdentifier()const;
		const std::vector<FunctionArg> getArgs() const;
		const TokenType_               getType() const;
		const std::string              getLabel() const;
		operator std::string() const;
	private:
		std::string label;
		std::string identifier;
		std::vector<FunctionArg> args;
		TokenType_ type;
	};

	/*
	 * This class links a function signature with an implementation. 
	 */
	class Function {
	public:

		Function(
			FunctionSignature _signature,
			FunctionImplem    _implementation):

			signature(_signature),
			implementation(_implementation)
		{}

		~Function() {}

		FunctionImplem    implementation;
		FunctionSignature signature;
	};
}