#pragma once

#include "Nodable.h"
#include "TokenType.h"
#include <functional>
#include <tuple>
#include <stdarg.h>     /* va_list, va_start, va_arg, va_end */

namespace Nodable {

	/*
	 * Type of a callable function.
	 * Require an integer as output (works like an error code, 0: OK, 1 >= : error)
	 * An output member and some arguments
     *
	 * TODO: try to replace Member* by Variant*
	 */
	typedef std::function <int (Member*, const std::vector<Member*>&)> FunctionImplem;

	/*
	 * Simple object to store a function argument (token, name)
	 */
	class FunctionArg {
	public:
		FunctionArg(TokenType, std::string);
		TokenType type;
		std::string name;
	};

	/*
	 * Class to store a function signature.
	 * We can check if two function signature are matching using this->match(other)
	 */
	class FunctionSignature {
	public:
		FunctionSignature(std::string _identifier, TokenType _type, std::string _label = "");
		~FunctionSignature() {};
		void                           pushArg(TokenType _type, std::string _name = "");

		template <typename... TokenType>
		void pushArgs(TokenType&&... args) {
			int dummy[] = { 0, ((void)pushArg(std::forward<TokenType>(args)),0)... };
		}
        bool                           hasAtLeastOneArgOfType(TokenType type);
		bool                           match(const FunctionSignature& _other)const;
		const std::string&             getIdentifier()const;
		std::vector<FunctionArg>       getArgs() const;
		TokenType                      getType() const;
		std::string                    getLabel() const;

	private:
		std::string label;
		std::string identifier;
		std::vector<FunctionArg> args;
		TokenType type;

	public:
		template<typename R = TokenType, typename... TokenType>
		static FunctionSignature Create(R _type, std::string _identifier, TokenType&& ..._args) {
			FunctionSignature signature(_identifier, _type);
			signature.pushArgs(_args...);
			return signature;
		}
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

		static void CheckArgumentsAndLogWarnings(const std::vector<Member*>& _args);
	};
}