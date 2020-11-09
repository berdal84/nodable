#pragma once

// std
#include <map>
#include <functional>
#include <tuple>
#include <regex>

// Nodable
#include "Nodable.h"   // for constants and forward declarations
#include "Function.h"
#include "Operator.h"
#include "TokenType.h"
#include "Dictionnary.h"
#include "Type.h"
#include "Log.h"

/*
	Here, some Macros to easily create function and add them to the Language.api

	TODO: I don't like these big macros. Use function traits instead.
*/
#define SUCCESS return 0;

#define FAIL \
_result->setType(Type::Any); /* We intentionnaly force result type any to avoid crashing.*/ \
return 1;

#define ARG(n) (*_args[n])
#define BEGIN_IMPL\
	auto implementation = [](Member* _result, const std::vector<Member*>& _args)->int { \
	for(auto it = _args.begin(); it != _args.end(); it++) \
	{\
		if( (*it)->isType(Type::Any) ) \
		{\
			LOG_WARNING( 1u, "Argument %i (%s) is unknown.\n", std::distance(_args.begin(), it), (*it)->getName().c_str() );\
		}\
	}

#define RETURN( expr )\
	_result->set( expr );


#define END_IMPL SUCCESS\
	};

#define BINARY_OP_BEGIN( _type, _identifier, _ltype, _rtype, _precedence, _label )\
{\
	auto precedence = _precedence; \
	auto identifier = std::string(_identifier); \
	auto signature  = std::make_shared<FunctionSignature>( std::string("operator") + _identifier, _type, _label ); \
	signature.pushArgs(_ltype, _rtype); \
	BEGIN_IMPL

#define UNARY_OP_BEGIN( _type, _identifier, _ltype, _precedence, _label )\
{\
	auto precedence = _precedence; \
	auto identifier = std::string(_identifier); \
	auto signature  = std::make_shared<FunctionSignature>( std::string("operator") + _identifier, _type, _label ); \
	signature.pushArgs(_ltype); \
	BEGIN_IMPL

#define FCT_BEGIN( _type, _identifier, ... ) \
{ \
	auto signature = FunctionSignature::Create( _type, _identifier, __VA_ARGS__); \
	BEGIN_IMPL

#define FCT_END \
	END_IMPL \
	addToAPI( signature, implementation );\
}

#define OPERATOR_END \
	END_IMPL \
	addOperator(identifier, precedence, signature, implementation);\
	addToAPI( signature , implementation );\
}

namespace Nodable {


	/*
		The role of this class is to define an interface for all languages in Nodable.

		using this class we can:
		- serialize tokens, functions (calls and signatures) an operators (binary and unary).
		- convert TokenType (abstract) to Type (Nodable types)
		- create, add and find functions.
		- create, add and find operators.

	 */
	class Language {
	public:

		Language(std::string _name): name(_name){};
		~Language() {};

		/* Serialize a function call with a signature and some values */
		virtual std::string                   serialize(const FunctionSignature&, std::vector<std::shared_ptr<Member>>)const = 0;

		/* Serialize a function signature */
		virtual std::string                   serialize(const FunctionSignature&)const = 0;

		/* Serialize a TokenType		
		   ex:
		   TokenType::LBracket => "("
		   TokenType::StringType = > "std::string" (for C++) */
		virtual std::string                   serialize(const TokenType&)const = 0;

		/* Serialize a binary operation call using an operator and two operands.
		   The last two operators are the source operators that creates the two operands as result.
		   Those are used to check precedence and add some brackets if needed.
		*/
		virtual std::string                   serializeBinaryOp(std::shared_ptr<const Operator>, std::vector<std::shared_ptr<Member>>, std::shared_ptr<const Operator>, std::shared_ptr<const Operator>)const = 0;
		virtual std::string                   serializeUnaryOp(std::shared_ptr<const Operator>, std::vector<std::shared_ptr<Member>>, std::shared_ptr<const Operator>)const = 0;
		virtual const FunctionSignature       createBinOperatorSignature(Type, std::string, Type, Type) const = 0;
		virtual const FunctionSignature       createUnaryOperatorSignature(Type, std::string, Type) const = 0;
		virtual const TokenType               typeToTokenType(Type _type)const = 0;
		virtual const Type                    tokenTypeToType(TokenType _tokenType)const = 0;

		void                                  addOperator(std::shared_ptr<Operator>);
		void                                  addOperator(std::string       _identifier,
			                                              unsigned short    _precedence,
			                                              FunctionSignature _prototype,
			                                              FunctionImplem  _implementation);
        std::shared_ptr<const Function>       findFunction(const FunctionSignature& signature) const;
		std::shared_ptr<const Operator>       findOperator(const FunctionSignature& _operator) const;
        std::shared_ptr<const Operator>       findOperator(const std::string& _identifier) const;
		void                                  addToAPI(const std::shared_ptr<Function>&);
		void                                  addToAPI(const std::shared_ptr<FunctionSignature>&, const FunctionImplem&);
		bool                                  hasHigherPrecedenceThan(const std::shared_ptr<const Operator>& _firstOperator, std::shared_ptr<const Operator> _secondOperator)const;
		[[nodiscard]] const std::vector< std::shared_ptr<Function> >& getAllFunctions()const { return api; }
		

		/**
		  * To generate the Nodable Language reference
		  * New language generators will be found here later...
		  * ex: static const Language* TypeScript();
		  */
		static std::shared_ptr<const Language> Nodable();


	private:
		/* Some language reference constants*/
		static std::shared_ptr<const Language> NODABLE;

	public:
		Dictionnary dictionnary;
	private:
		std::string name;
		std::vector< std::shared_ptr<Operator> > operators;
		std::vector< std::shared_ptr<Function> > api;
	};

}
