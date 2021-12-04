#pragma once

// std
#include <map>
#include <functional>
#include <tuple>
#include <regex>

// Nodable
#include <nodable/Nodable.h> // for constants and forward declarations
#include <nodable/TokenType.h>
#include <nodable/Type.h>
#include <nodable/Log.h>
#include <nodable/Function.h>
#include <nodable/Semantic.h>
#include <nodable/Operator.h>
#include <nodable/Language_MACROS.h>
#include <nodable/Serializer.h>
#include <nodable/Parser.h>

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

        const Invokable* findFunction(const FunctionSignature* signature) const;
        const Operator* findOperator(const FunctionSignature* _operator) const;
        const Operator* findOperator(const std::string& _short_identifier) const;

        inline Parser* getParser()const { return parser; }
        inline Serializer* getSerializer()const { return serializer; }
        inline const Semantic* getSemantic()const { return &semantic; }
        inline const std::vector<Invokable*>& getAllFunctions()const { return api; }

        const FunctionSignature* createUnaryOperatorSignature(Type _type, std::string _identifier, Type _ltype) const;
        const FunctionSignature* createBinOperatorSignature(Type _type, std::string _identifier, Type _ltype, Type _rtype) const;

        bool hasHigherPrecedenceThan(const Operator *_firstOperator, const Operator* _secondOperator)const;
        virtual void sanitizeFunctionName( std::string& identifier ) const = 0;
        virtual void sanitizeOperatorFunctionName( std::string& identifier ) const = 0;
	protected:
        void addOperator(Operator*);
        void addToAPI(Invokable*);

        Semantic semantic;
        Serializer* serializer;
        Parser* parser;

	private:
		std::string name;
		std::vector<Operator*> operators;
		std::vector<Invokable*> api;
    };

}
