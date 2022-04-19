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
#include <nodable/core/languages/NodableSerializer.h>
#include <nodable/core/languages/NodableParser.h>
#include <nodable/core/Operator.h>

namespace Nodable {

    template<typename T>
    struct signature_builder;

    class IParser;
    class ISerializer;

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
	class ILanguage {
	    using vec_operator_t  = std::vector<const Operator*>;
	    using vec_invokable_t = std::vector<const IInvokable*>;
	public:
		virtual ~ILanguage() = default;

        virtual const IInvokable*               find_function(const Signature*) const = 0;
        virtual const IInvokable*               find_operator_fct(const Signature*) const = 0;
        virtual const IInvokable*               find_operator_fct_exact(const Signature*) const = 0;
        virtual const Operator*                 find_operator(const std::string& , Operator_t) const = 0;
        virtual IParser&                        get_parser() = 0;
        virtual const IParser&                  get_parser()const = 0;
        virtual const ISerializer&              get_serializer()const = 0;
        virtual const vec_invokable_t&          get_api()const  = 0;
        virtual const std::vector<std::regex>&  get_token_type_regex()const  = 0;
        virtual std::string&                    to_string(std::string&, type)const = 0;
        virtual std::string&                    to_string(std::string&, Token_t)const = 0;
        virtual std::string                     to_string(type)const = 0;
        virtual std::string                     to_string(Token_t)const = 0;
        virtual type                            get_type(Token_t _token)const = 0;
        virtual const std::vector<Token_t>&     get_token_type_regex_index_to_token_type()const = 0;
        virtual const Signature*                new_operator_signature(type, const Operator*, type)const = 0;
        virtual const Signature*                new_operator_signature(type, const Operator*, type, type)const = 0;
        virtual std::string                     sanitize_function_id(const std::string& _id)const = 0;
        virtual std::string                     sanitize_operator_id(const std::string& _id)const = 0;
    };

    /**
     * Builder to create function/operator signatures for a given language
     * @tparam T is the function's return type
     * @tparam Args is the function's argument(s) type
     *
     * usage: Signature* sig = signature-builder<double(double,double)>::signature()
     *                                                                  .with_id("+")
     *                                                                  .as_operator()
     *                                                                  .with_language(lang_ptr).build();
     */
    template<typename T, typename ...Args>
    struct signature_builder<T(Args...)>
    {
        using this_t = signature_builder<T(Args...)>;

        std::string       m_id;
        const ILanguage*  m_language;
        bool              m_as_operator;

        signature_builder()
                : m_as_operator(false)
                , m_language(nullptr){}

        void with_id(const std::string& _id) { m_id = _id; }
        void with_lang(const ILanguage* _language) { m_language = _language; }
        void as_operator(bool _b = true) { m_as_operator = _b; }

        Signature* construct()
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
