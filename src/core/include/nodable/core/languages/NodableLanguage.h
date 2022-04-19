#pragma once
#include <nodable/core/ILanguage.h>
#include <nodable/core/System.h>

namespace Nodable {

	/*
		The role of this class is to define a first language for Nodable.
	*/
	class NodableLanguage : public ILanguage
	{
	public:
        using Operators_t          = std::vector<const Operator*>;
        using InvokableFunctions_t = std::vector<const IInvokable*>;

		NodableLanguage();
        ~NodableLanguage() override;

        std::string                     sanitize_function_id(const std::string& _id)const override;
        std::string                     sanitize_operator_id(const std::string& _id)const override;
        const IInvokable*               find_function(const Signature*) const override;
        const IInvokable*               find_operator_fct(const Signature*) const override;
        const IInvokable*               find_operator_fct_exact(const Signature*) const override;
        const Operator*                 find_operator(const std::string& , Operator_t) const override;
        IParser&                        get_parser() override { return m_parser; }
        const IParser&                  get_parser()const override { return m_parser; }
        const ISerializer&              get_serializer()const override { return m_serializer; }
        const InvokableFunctions_t&     get_api()const override { return m_functions; }
        const std::vector<std::regex>&  get_token_type_regex()const override { return m_token_regex;  }
        std::string&                    to_string(std::string&, type)const override;
        std::string&                    to_string(std::string&, Token_t)const override;
        std::string                     to_string(type)const override;
        std::string                     to_string(Token_t)const override;
        type                            get_type(Token_t _token)const override { return m_token_to_type.find(_token)->second; }
        const std::vector<Token_t>&     get_token_type_regex_index_to_token_type()const override { return m_regex_to_token; }
        const Signature*                new_operator_signature(type, const Operator*, type)const override;
        const Signature*                new_operator_signature(type, const Operator*, type, type)const override;
        template<typename T>
        Signature*                      new_function_signature(const std::string& _id)const
        {
            signature_builder<T> builder;
            builder.with_id(_id);
            builder.with_lang(this);
            return builder.construct();
        }

        template<typename T>
        Signature*                      new_operator_signature(const std::string& _id)const
        {
            signature_builder<T> builder;
            builder.with_id(_id);
            builder.with_lang(this);
            builder.as_operator();
            return builder.construct();
        }
    private:
        void                            add_invokable(const IInvokable*);
        void                            add_operator(const char* _id, Operator_t _type, int _precedence);
        const IInvokable*               find_operator_fct_fallback(const Signature*) const;
        void                            add_regex(const std::regex &_regex, Token_t _token_t);
        void                            add_regex(const std::regex &_regex, Token_t _token_t, type _type);
        void                            add_string(std::string _string, Token_t _token_t);
        void                            add_type(type _type, Token_t _token_t, std::string _string);
        void                            add_type(type _type, std::string _string);
        void                            add_char(const char _char, Token_t _token_t);

        NodableSerializer               m_serializer;
        NodableParser                   m_parser;
        Operators_t              m_operators;
        InvokableFunctions_t     m_operator_implems;
        InvokableFunctions_t     m_functions;
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
	public:
        static constexpr const char* k_keyword_operator   = "operator";
        static constexpr const char* k_keyword_if         = "if";
        static constexpr const char* k_keyword_else       = "else";
        static constexpr const char* k_keyword_for        = "for";
        static constexpr const char* k_keyword_bool       = "bool";
        static constexpr const char* k_keyword_string     = "string";
        static constexpr const char* k_keyword_double     = "double";
        static constexpr const char* k_keyword_int        = "int";
        static constexpr const char  k_tab                = '\t';
        static constexpr const char  k_space              = ' ';
        static constexpr const char  k_open_curly_brace   = '{';
        static constexpr const char  k_close_curly_brace  = '}';
        static constexpr const char  k_open_bracket       = '(';
        static constexpr const char  k_close_bracket      = ')';
        static constexpr const char  k_coma               = ',';
        static constexpr const char  k_semicolon          = ';';
        static constexpr const char  k_end_of_line        = System::k_end_of_line;
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

