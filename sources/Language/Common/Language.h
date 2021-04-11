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
#include "Semantic.h"
#include "Operator.h"
#include "Language_MACROS.h"
#include <Language/Common/Serializer.h>

namespace Nodable {

    // forward declarations
    class Parser;

	/**
		The role of this class is to define an interface for all languages in Nodable.

		using this class we can:
		- serialize tokens, functions (calls and signatures) an operators (binary and unary).
		- convert TokenType (abstract) to Type (Nodable types)
		- create, add and find functions.
		- create, add and find operators.

	 */
	class Language {
	public:

		Language(
		        std::string _name,
                Parser* _parser,
                Serializer* _serializer
                )
                :
                name(_name),
		        serializer(_serializer),
		        parser(_parser)
        {
        };

		virtual ~Language();

        const Function* findFunction(const FunctionSignature& signature) const;
        const Operator* findOperator(const FunctionSignature& _operator) const;
        const Operator* findOperator(const std::string& _identifier) const;

        inline Parser* getParser()const { return parser; }
        inline Serializer* getSerializer()const { return serializer; }
        inline const Semantic* getSemantic()const { return &semantic; }
        inline const std::vector<Function>& getAllFunctions()const { return api; }

        const FunctionSignature createUnaryOperatorSignature(Type _type, std::string _identifier, Type _ltype) const;
        const FunctionSignature createBinOperatorSignature(Type _type, std::string _identifier, Type _ltype, Type _rtype) const;

        bool hasHigherPrecedenceThan(const Operator *_firstOperator, const Operator* _secondOperator)const;

	protected:
        void addOperator(Operator);
        void addOperator(std::string _identifier, unsigned short    _precedence, FunctionSignature _prototype, FunctionImplem  _implementation);
        void addToAPI(Function);
        void addToAPI(FunctionSignature&, FunctionImplem);

        Semantic semantic;
        Serializer* serializer;
        Parser* parser;

	private:
		std::string name;
		std::vector<Operator> operators;
		std::vector<Function> api;
    };

}
