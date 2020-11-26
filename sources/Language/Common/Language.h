#pragma once

// std
#include <map>
#include <functional>
#include <tuple>
#include <regex>
#include <Component/ComputeUnaryOperation.h>

// Nodable
#include "Nodable.h"   // for constants and forward declarations
#include "TokenType.h"
#include "Type.h"
#include "Log.h"
#include "Function.h"
#include "Dictionnary.h"
#include "Operator.h"
#include "Language_MACROS.h"
#include <Language/Common/Serializer.h>

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

		Language(std::string _name, Serializer* _serializer):
		    name(_name),
		    serializer(_serializer)
        {
        };

		virtual ~Language() = default;

        void                                  addOperator(Operator);
        void                                  addOperator(std::string       _identifier,
                                                          unsigned short    _precedence,
                                                          FunctionSignature _prototype,
                                                          FunctionImplem  _implementation);
        const Function*                       findFunction(const FunctionSignature& signature) const;
        const Operator*                       findOperator(const FunctionSignature& _operator) const;
        const Operator*                       findOperator(const std::string& _identifier) const;
        void                                  addToAPI(Function);
        void                                  addToAPI(FunctionSignature&, FunctionImplem);
        bool                                  hasHigherPrecedenceThan(const Operator *_firstOperator, const Operator* _secondOperator)const;
        const std::vector<Function>&          getAllFunctions()const { return api; }

        virtual const FunctionSignature       createBinOperatorSignature(Type, std::string, Type, Type) const = 0;
		virtual const FunctionSignature       createUnaryOperatorSignature(Type, std::string, Type) const = 0;
		virtual const TokenType               typeToTokenType(Type _type)const = 0;
		virtual const Type                    tokenTypeToType(TokenType _tokenType)const = 0;

		[[nodiscard]] inline const Serializer* getSerializer()const { return serializer; }

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
		Dictionnary dictionnary;
	private:
		std::string name;
		std::vector<Operator> operators;
		std::vector<Function> api;
		const Serializer* serializer;
    };

}
