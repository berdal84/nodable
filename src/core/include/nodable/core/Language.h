#pragma once

// std
#include <map>
#include <functional>
#include <tuple>
#include <regex>
#include <memory> // std::shared_ptr

// Nodable
#include <nodable/core/types.h> // for constants and forward declarations
#include <nodable/core/Token_t.h>
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
	    using Meta_t      = std::shared_ptr<const R::MetaType>;
	    using Operators_t = std::vector<const Operator*>;
	    using InvokableFunctions_t = std::vector<const IInvokable*>;
	public:

		Language(const char* _name, Parser* _parser, Serializer* _serializer)
                : m_name(_name), m_parser(_parser), m_serializer(_serializer)
        {
        };

		virtual ~Language();

        const IInvokable*               find_function(const FunctionSignature*) const;
        const InvokableOperator*        find_operator_fct(const FunctionSignature*) const;
        const InvokableOperator*        find_operator_fct_exact(const FunctionSignature*) const;
        const Operator*                 find_operator(const std::string& , Operator_t) const;
        const Operator*                 find_operator(const std::string& _identifier, const FunctionSignature* _signature) const;

        Parser*                         get_parser()const { return m_parser; }
        Serializer*                     get_serializer()const { return m_serializer; }
        const Semantic*                 get_semantic()const { return &m_semantic; }
        const InvokableFunctions_t&     get_api()const { return m_functions; }

        const FunctionSignature*        new_unary_operator_signature(Meta_t , std::string , Meta_t ) const;
        const FunctionSignature*        new_bin_operator_signature(Meta_t, std::string , Meta_t , Meta_t ) const;

        bool                            has_higher_precedence_than(std::pair<const InvokableOperator*, const InvokableOperator*> _operators)const;
        virtual void                    sanitize_function_identifier( std::string& ) const = 0;
        virtual void                    sanitize_operator_fct_identifier( std::string& identifier ) const = 0;
	protected:
        void                            add(const IInvokable*);
        void                            add(const Operator*);
        void                            add(const InvokableOperator*);
        const InvokableOperator*        find_operator_fct_fallback(const FunctionSignature*) const;

        Semantic     m_semantic;
        Serializer*  m_serializer;
        Parser*      m_parser;

	private:
		std::string m_name;
		Operators_t m_operators;
        InvokableFunctions_t m_operator_implems;
		InvokableFunctions_t m_functions;
    };

}
