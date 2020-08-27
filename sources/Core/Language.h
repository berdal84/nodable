#pragma once

#include "Nodable.h"   // for constants and forward declarations
#include "Function.h"
#include "Operator.h"
#include "LanguageEnums.h"
#include <map>
#include <functional>

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
		std::map<std::string, TokenType_> keywords;
	private:
		std::string name;
		std::vector<char> brackets;
		std::map<std::string, Operator> operators;
		std::vector<Function> api;
	};

}
