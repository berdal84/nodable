#pragma once

#include "Nodable.h"
#include "TokenType.h"
#include <functional>
#include <tuple>
#include <utility>
#include <stdarg.h>     /* va_list, va_start, va_arg, va_end */
#include <Core/Member.h>

namespace Nodable {

	/*
	 * Type of a callable function.
	 * Require an integer as output (works like an error code, 0: OK, 1 >= : error)
	 * An output member and some arguments
     *
	 * TODO: try to replace Member* by Variant*
	 */
	typedef std::function <int (std::shared_ptr<Member>, const std::vector<std::shared_ptr<Member>>&)> FunctionImplem;

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

		bool                           match(const std::shared_ptr<const FunctionSignature>& _other)const;
		const std::string&             getIdentifier()const;
		const std::vector<FunctionArg> getArgs() const;
		const TokenType               getType() const;
		const std::string              getLabel() const;

	private:
		std::string label;
		std::string identifier;
		std::vector<FunctionArg> args;
		TokenType type;

	public:
		template<typename R = TokenType, typename... TokenType>
		static const std::shared_ptr<FunctionSignature> Create(R _type, std::string _identifier, TokenType&& ..._args) {
			auto signature = std::make_shared<FunctionSignature>( _identifier, _type );
			signature->pushArgs(_args...);
			return signature;
		}
	};

	

	/*
	 * This class links a function signature with an implementation. 
	 */
	class Function {
	public:

		Function(
            std::shared_ptr<FunctionSignature>  _signature,
            FunctionImplem   _implementation):

			signature(std::move(_signature)),
			implementation(std::move(_implementation))
		{}

		~Function() {}

        const FunctionImplem implementation;
        const std::shared_ptr<FunctionSignature> signature;
	};
}