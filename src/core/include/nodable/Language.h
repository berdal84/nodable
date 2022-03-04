#pragma once

// std
#include <map>
#include <functional>
#include <tuple>
#include <regex>

// Nodable
#include <nodable/Nodable.h> // for constants and forward declarations
#include <nodable/TokenType.h>
#include <nodable/R.h>
#include <nodable/Log.h>
#include <nodable/InvokableFunction.h>
#include <nodable/Semantic.h>
#include <nodable/InvokableOperator.h>
#include <nodable/Language_MACROS.h>
#include <nodable/Serializer.h>
#include <nodable/Parser.h>

namespace Nodable {

    // forward declarations
    class Parser;

	/**
	 * @brief The role of this class is to define a base abstract class for all languages.
	 *
	 * Using this class we can:
	 * - serialize tokens, functions (calls and signatures) an operators (binary and unary).
	 * - convert TokenType (abstract) to Type (Nodable types)
	 * - create, add and find functions.
	 * - create, add and find operators.
	 * - etc.
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

        const IInvokable* findFunction(const FunctionSignature* signature) const;
        const InvokableOperator* findOperator(const FunctionSignature* _operator) const;
        const InvokableOperator* findOperator(const std::string& _short_identifier) const;

        inline Parser* getParser()const { return parser; }
        inline Serializer* getSerializer()const { return serializer; }
        inline const Semantic* getSemantic()const { return &semantic; }
        inline const std::vector<IInvokable*>& getAllFunctions()const { return api; }

        const FunctionSignature* createUnaryOperatorSignature(const R::Type* , std::string , const R::Type* ) const;
        const FunctionSignature* createBinOperatorSignature(const R::Type* , std::string , const R::Type* , const R::Type* ) const;

        bool hasHigherPrecedenceThan(const InvokableOperator *_firstOperator, const InvokableOperator* _secondOperator)const;
        virtual void sanitizeFunctionName( std::string& identifier ) const = 0;
        virtual void sanitizeOperatorFunctionName( std::string& identifier ) const = 0;
	protected:
        void addOperator(InvokableOperator*);
        void addToAPI(IInvokable*);

        Semantic semantic;
        Serializer* serializer;
        Parser* parser;

	private:
		std::string name;
		std::vector<InvokableOperator*> operators;
		std::vector<IInvokable*> api;
    };

}
