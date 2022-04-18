#pragma once

// std
#include <map>
#include <unordered_map>
#include <functional>
#include <tuple>
#include <vector>
#include <regex>
#include <memory> // std::shared_ptr
#include <string>

// Nodable
#include <nodable/core/types.h> // for constants and forward declarations
#include <nodable/core/Token_t.h>
#include <nodable/core/reflection/reflection>
#include <nodable/core/Log.h>
#include <nodable/core/Invokable.h>
#include <nodable/core/Language_MACROS.h>
#include <nodable/core/Serializer.h>
#include <nodable/core/Parser.h>
#include <nodable/core/Operator.h>

namespace Nodable {

    template<typename T>
    struct signature_builder;

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
        const InvokableFunctions_t&     get_api()const { return m_functions; }
        const std::vector<std::regex>&  get_token_type_regex()const { return m_token_regex;  }
        std::string&                    to_string(std::string&, type)const;
        std::string&                    to_string(std::string&, Token_t)const;
        std::string                     to_string(type)const;
        std::string                     to_string(Token_t)const;
        type                            get_type(Token_t _token)const { return m_token_to_type.find(_token)->second; }
        const std::vector<Token_t>&     get_token_type_regex_index_to_token_type()const { return m_regex_to_token; }

        const Signature*                new_operator_signature(type, const Operator*, type)const;
        const Signature*                new_operator_signature(type, const Operator*, type, type)const;
        virtual std::string             sanitize_function_id(const std::string& _id)const = 0;
        virtual std::string             sanitize_operator_id(const std::string& _id)const = 0;

        template<typename T>
        Signature*                      new_function_signature(const std::string& _id)const
        {
            return signature_builder<T>::signature().with_id(_id).with_lang(this).build();
        }

        template<typename T>
        Signature*                      new_operator_signature(const std::string& _id)const
        {
            return signature_builder<T>::signature().with_id(_id).as_operator().with_lang(this).build();
        }

    protected:
        void                            add_invokable(const IInvokable*);
        void                            add_operator(const char* _id, Operator_t _type, int _precedence);
        const IInvokable*               find_operator_fct_fallback(const Signature*) const;
        void                            add_regex(const std::regex &_regex, Token_t _token_t);
        void                            add_regex(const std::regex &_regex, Token_t _token_t, type _type);
        void                            add_string(std::string _string, Token_t _token_t);
        void                            add_type(type _type, Token_t _token_t, std::string _string);
        void                            add_type(type _type, std::string _string);
        void                            add_char(const char _char, Token_t _token_t);

        Serializer               m_serializer;
        Parser                   m_parser;

        // indexes and regexes...
        std::vector<std::regex>  m_type_regex;
        std::vector<std::regex>  m_token_regex;
        std::vector<Token_t>     m_regex_to_token;
        std::vector<type>        m_regex_to_type;
        std::unordered_map<size_t, Token_t>      m_type_to_token;
        std::unordered_map<Token_t, type>        m_token_to_type;
        std::unordered_map<Token_t, std::string> m_token_to_string;
        std::unordered_map<size_t, std::string>  m_type_to_string;
        std::unordered_map<Token_t, const char>  m_token_to_char;
        std::unordered_map<size_t, Token_t>      m_char_to_token;
	private:
		std::string          m_name;
		Operators_t          m_operators;
        InvokableFunctions_t m_operator_implems;
		InvokableFunctions_t m_functions;

    };

    template<typename T, typename ...Args>
    struct signature_builder<T(Args...)>
    {
        using this_t = signature_builder<T(Args...)>;

        std::string      m_id;
        const Language*  m_language;
        bool             m_as_operator;

        signature_builder()
                : m_as_operator(false)
                , m_language(nullptr){}

        static this_t signature() { return signature_builder(); }
        this_t with_id(const std::string& _id) { m_id = _id; return *this;}
        this_t with_lang(const Language* _language) { m_language = _language; return *this;}
        this_t as_operator() { m_as_operator = true; return *this;}

        Signature* build()
        {
            NODABLE_ASSERT_EX( m_language   , "No language specified! use with_lang()" );
            NODABLE_ASSERT_EX( !m_id.empty(), "No identifier specified! use with_id()" );

            Signature* signature;
            if( m_as_operator )
            {
                const Operator* op;
                size_t argc = sizeof...(Args);
                switch ( argc )
                {
                    case 1:  op = m_language->find_operator(m_id, Operator_t::Unary);  break;
                    case 2:  op = m_language->find_operator(m_id, Operator_t::Binary); break;
                    default: op = nullptr;
                }
                NODABLE_ASSERT_EX( op, "No operator found  in language for specified id!" );
                signature = new Signature(m_language->sanitize_operator_id(op->identifier), op);
            }
            else
            {
                m_id = m_language->sanitize_function_id(m_id);
                NODABLE_ASSERT_EX( !m_id.empty(), "Identifier after sanitization is empty!" );
                signature = new Signature( m_id );
            }

            signature->set_return_type(type::get<T>());
            signature->push_args<std::tuple<Args...>>();

            return signature;
        }
    };

}
