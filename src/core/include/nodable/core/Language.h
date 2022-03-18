#pragma once

// std
#include <map>
#include <functional>
#include <tuple>
#include <regex>
#include <memory> // std::shared_ptr

// Nodable
#include <nodable/core/types.h> // for constants and forward declarations
#include <nodable/core/TokenType.h>
#include <nodable/core/reflection/R.h>
#include <nodable/core/Log.h>
#include <nodable/core/InvokableFunction.h>
#include <nodable/core/Semantic.h>
#include <nodable/core/InvokableOperator.h>
#include <nodable/core/Language_MACROS.h>
#include <nodable/core/Serializer.h>
#include <nodable/core/Parser.h>

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

        const FunctionSignature* createUnaryOperatorSignature(std::shared_ptr<const R::MetaType> , std::string , std::shared_ptr<const R::MetaType> ) const;
        const FunctionSignature* createBinOperatorSignature(std::shared_ptr<const R::MetaType>, std::string , std::shared_ptr<const R::MetaType> , std::shared_ptr<const R::MetaType> ) const;

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
