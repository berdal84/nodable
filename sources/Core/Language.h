#pragma once

#include "Nodable.h"   // for constants and forward declarations
#include "Function.h"
#include "Operator.h"
#include "LanguageEnums.h"
#include <map>
#include <functional>
#include <tuple>
#include <regex>


// Some Macros to easily create function and add them to the Language.api

#define ARG(n) *_args[n]
#define BEGIN_IMPL\
	auto implementation = [](Member* _result, const std::vector<const Member*>& _args)->int {

#define RETURN( expr )\
	_result->set( expr );

#define SUCCESS return 0;
#define FAIL return 1;
#define END_IMPL SUCCESS\
	};

#define OPERATOR_BEGIN( _ltype, _identifier, _rtype, _precedence, _type, _label )\
{\
	auto precedence = _precedence; \
	auto identifier = std::string(_identifier); \
	FunctionSignature signature( std::string("operator") + _identifier, _type, _label ); \
	signature.pushArgs(_ltype, _rtype); \
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
}

namespace Nodable {


	/*
	 * This class store all basic informations to store a language:
	 * operators with precedence, functions and API
	 */
	class Language {
	public:

		Language(std::string _name) :numbers(), letters(), brackets(), name(_name){};
		~Language() {};

		virtual std::string                   serialize(const FunctionSignature&, std::vector<const Member*>)const = 0;
		virtual std::string                   serialize(const FunctionSignature&)const = 0;
		virtual std::string                   serialize(const TokenType&)const = 0;

		void                                  addOperator(Operator);
		void                                  addOperator(std::string       _identifier,
			                                              unsigned short    _precedence,
			                                              FunctionSignature _prototype,
			                                              FunctionImplem  _implementation);
		unsigned short                        getOperatorPrecedence(const std::string& _identifier)const;
		std::string                           getOperatorsAsString()const;
		const Function*                       findFunction(FunctionSignature& signature) const;
		const Operator*                       findOperator(const std::string& _operator) const;
		void                                  addToAPI(Function);
		void                                  addToAPI(FunctionSignature&, FunctionImplem);
		bool                                  needsToBeEvaluatedFirst(std::string op, std::string nextOp)const;
		const std::vector<Function>&          getAPI()const { return api; }		
		

		/**
		  * To generate the Nodable Language reference
		  * New language generators will be found here later...
		  * ex: static const Language* TypeScript();
		  */
		static const Language* Nodable();


	private:
		/* Some language reference constants*/
		static const Language* NODABLE;

	public:
		std::string numbers;
		std::string letters;
		std::map<std::string, TokenType> keywordToTokenType;
		std::map<TokenType, std::string> tokenTypeToString;
		std::map<TokenType, std::regex>  tokenTypeToRegex;
	private:
		std::string name;
		std::vector<char> brackets;
		std::map<std::string, Operator> operators;
		std::vector<Function> api;
	};

}
