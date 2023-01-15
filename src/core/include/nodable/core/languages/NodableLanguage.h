#pragma once
#include <nodable/core/Operator_t.h>
#include <nodable/core/System.h>
#include <nodable/core/languages/NodableParser.h>
#include <nodable/core/languages/NodableSerializer.h>
#include <nodable/core/reflection/func_type.h>// usage in template function load_library

namespace ndbl {

	/**
	 * Nodable Language implementation
	 */
	class NodableLanguage
	{
	public:
        using Operators_t          = std::vector<const Operator*>;
        using InvokableFunctions_t = std::vector<std::shared_ptr<const iinvokable>>;

		NodableLanguage();
        ~NodableLanguage();

        std::shared_ptr<const iinvokable> find_function(const func_type*) const;                 // Find a function by signature.
        std::shared_ptr<const iinvokable> find_operator_fct(const func_type*) const;             // Find an operator's function by signature (casts allowed).
        std::shared_ptr<const iinvokable> find_operator_fct_exact(const func_type*) const;       // Find an operator's function by signature (strict mode, no cast allowed).
        const Operator*                 find_operator(const std::string& , Operator_t) const;    // Find an operator by symbol and type (unary, binary or ternary).
        NodableParser&                  get_parser() { return m_parser; }                        // Get a parser for that language (TODO: Parser, Serialized, Language could be merged).
        const NodableParser&            get_parser()const { return m_parser; }                   // Get a const parser (for operation not requiring a state).
        const NodableSerializer&        get_serializer()const { return m_serializer; }           // Get a serializer for that language.
        const InvokableFunctions_t&     get_api()const { return m_functions; }                   // Get all the functions registered in the language. (TODO: why do we store the declared functions here? can't we load them in the VirtualMachine instead?).
        const std::vector<std::regex>&  get_token_regexes()const { return m_token_regex;  }      // Get all the regexes to convert a string to the corresponding token_t.
        std::string&                    to_string(std::string& /*out*/, type)const;              // Convert a type to string (by ref).
        std::string&                    to_string(std::string& /*out*/, Token_t)const;           // Convert a type to a token_t (by ref).
        std::string                     to_string(type)const;                                    // Convert a type to string.
        std::string                     to_string(Token_t)const;                                 // Convert a type to a token_t.
        type                            get_type(Token_t _token)const;                           // Get the type corresponding to a given token_t (must be a type keyword)
        const std::vector<Token_t>&     get_token_t_by_regex_index()const { return m_token_t_by_regex_index; }
        void                            add_function(std::shared_ptr<const iinvokable>);         // Adds a new function (regular or operator's implementation).
        int                             get_precedence(const iinvokable*)const;                  // Get the precedence of a given function (precedence may vary because function could be an operator implementation).

        template<typename T>
        void load_library()             // Instantiate a library from its type (uses reflection to get all its static methods).
        {
            T library; // will force static code to run

            auto type = type::get<T>();
            for(auto& each_static : type.get_static_methods())
            {
                add_function(each_static);
            }
        }
    private:
        void                            add_operator(const char*, Operator_t, int _precedence);  // Add a new (abstract) operator (ex: "==", Operator_t::binary, 5)
        std::shared_ptr<const iinvokable> find_operator_fct_fallback(const func_type*) const;    // Find a fallback operator function for a given signature (allows cast).
        void                            add_regex(const std::regex&, Token_t);                   // Register a regex to token_t couple.
        void                            add_regex(const std::regex&, Token_t, type);             // Register a regex, token_t, type tuple.
        void                            add_string(std::string, Token_t);                        // Register a string to token_t couple. ( "<<" => Token_t::operator )
        void                            add_char(char, Token_t);                                 // Register a char to token_t couple ( '*' => Token_t::operator )
        void                            add_type(type, Token_t, std::string);                    // Register a type, token_t, string tuple (ex: type::get<int>, Token_t::keyword_int, "int")

        NodableSerializer        m_serializer;
        NodableParser            m_parser;
        Operators_t              m_operators;                                  // the allowed operators (!= implementations).
        InvokableFunctions_t     m_operators_impl;                             // operators' implementations.
        InvokableFunctions_t     m_functions;                                  // all the functions (including operator's).
        std::vector<std::regex>  m_type_regex;                                 // Regular expressions to get a type from a string.
        std::vector<type>        m_type_by_regex_index;                        // Types sorted by m_type_regex index.
        std::vector<std::regex>  m_token_regex;                                // Regular expressions to get a token from a string.
        std::vector<Token_t>     m_token_t_by_regex_index;                     // Token sorted by m_token_regex index.
        std::unordered_map<size_t, Token_t>      m_type_hashcode_to_token_t;   // type's hashcode into a token_t (ex: type::get<std::string>().hashcode() => Token_t::keyword_string)
        std::unordered_map<size_t, std::string>  m_type_hashcode_to_string;    // type's hashcode into a string (ex: type::get<std::string>().hashcode() => "std::string")
        std::unordered_map<Token_t, type>        m_token_type_keyword_to_type; // token_t to type. Works only if token_t refers to a type keyword.
        std::unordered_map<Token_t, const char>  m_token_t_to_char;            // token_t to single char (ex: Token_t::operator => '*').
        std::unordered_map<Token_t, std::string> m_token_t_to_string;          // token_t to string (ex: Token_t::keyword_double => "double").
        std::unordered_map<size_t, Token_t>      m_char_to_token_t;            // single char to token_t (ex: '*' => Token_t::operator).
	public:

        // token as string

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
}

