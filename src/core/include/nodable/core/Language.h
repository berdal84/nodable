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
#include <nodable/core/Invokable.h>
#include <nodable/core/Semantic.h>
#include <nodable/core/Language_MACROS.h>
#include <nodable/core/Serializer.h>
#include <nodable/core/Parser.h>
#include "Operator.h"

namespace Nodable {

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
	    using Operators_t          = std::vector<const Operator*>;
	    using InvokableFunctions_t = std::vector<const IInvokable*>;
	public:

		Language(const char* _name)
                : m_name(_name)
                , m_parser(this)
                , m_serializer(this)
        {
        };

		virtual ~Language();

        const IInvokable*               find_function(const Signature*) const;
        const IInvokable*               find_operator_fct(const Signature*) const;
        const IInvokable*               find_operator_fct_exact(const Signature*) const;
        const Operator*                 find_operator(const std::string& , Operator_t) const;

        Parser&                         get_parser() { return m_parser; }
        const Parser&                   get_parser()const { return m_parser; }
        const Serializer&               get_serializer()const { return m_serializer; }
        const Semantic&                 get_semantic()const { return m_semantic; }
        const InvokableFunctions_t&     get_api()const { return m_functions; }

    protected:
        void                            add_invokable(const IInvokable*);
        void                            add_operator(const char* _id, Operator_t _type, int _precedence);
        const IInvokable*               find_operator_fct_fallback(const Signature*) const;

        Semantic     m_semantic;
        Serializer   m_serializer;
        Parser       m_parser;

	private:
		std::string m_name;
		Operators_t          m_operators;
        InvokableFunctions_t m_operator_implems;
		InvokableFunctions_t m_functions;
    };


    template<typename T>
    struct FindOperator;

    template<typename T, typename ...Args>
    struct FindOperator<T(Args...)>
    {
        const char* id;

        FindOperator(const char* _id): id(_id)
        {
        }

        const Operator* in_language( const Language* _lang)
        {
            size_t argc = sizeof...(Args);
            switch ( argc )
            {
                case 1: return _lang->find_operator(id, Operator_t::Unary);
                case 2: return _lang->find_operator(id, Operator_t::Binary);
                default: NODABLE_ASSERT(false)
            }
        }
    };
}
