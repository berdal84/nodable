#pragma once
#include <nodable/core/Language.h>
#include <nodable/core/System.h>

namespace Nodable {

	/*
		The role of this class is to define a first language for Nodable.
	*/
	class LanguageNodable : public Language
	{
	public:
		LanguageNodable();
        ~LanguageNodable() override = default;

        std::string sanitize_function_id(const std::string& _id)const override;
        std::string sanitize_operator_id(const std::string& _id)const override;

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

