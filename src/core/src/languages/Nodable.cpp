#include <nodable/core/languages/Nodable.h>

#include <nodable/core/Member.h>
#include <nodable/core/Format.h>
#include <nodable/core/System.h>

#include <nodable/core/languages/Nodable_API.h> // <----- contains all functions referenced below

using namespace Nodable;
using namespace Nodable::R;
using string = std::string;

LanguageNodable::LanguageNodable()
    :
    Language("Nodable", new Parser(this), new Serializer(this))
{
    /*
     *  Configure the Semantic.
     *
     *  The order of insertion is important. First inserted will be taken in priority by Parser.
     */

    // ignored
    m_semantic.insert(std::regex("^(//(.+?)$)"), TokenType_Ignore);      // Single line
    m_semantic.insert(std::regex("^(/\\*(.+?)\\*/)"), TokenType_Ignore); // Multi line
    m_semantic.insert("\t", TokenType_Ignore);
    m_semantic.insert(" ", TokenType_Ignore);

    // keywords
    m_semantic.insert("if", TokenType_KeywordIf);                      // conditional structures
    m_semantic.insert("else", TokenType_KeywordElse);
    m_semantic.insert("for", TokenType_KeywordFor);
    m_semantic.insert("bool", TokenType_KeywordBoolean, Type::Boolean); // types
    m_semantic.insert("string", TokenType_KeywordString, Type::String);
    m_semantic.insert("double", TokenType_KeywordDouble, Type::Double);
    m_semantic.insert("int", TokenType_KeywordInt, Type::Int16);

    // punctuation
    m_semantic.insert("{", TokenType_BeginScope);
    m_semantic.insert("}", TokenType_EndScope);
    m_semantic.insert("(", TokenType_OpenBracket);
    m_semantic.insert(")", TokenType_CloseBracket);
    m_semantic.insert(",", TokenType_Separator);
    m_semantic.insert(";", TokenType_EndOfInstruction);
    m_semantic.insert(std::string{System::k_end_of_line}, TokenType_EndOfLine);

    // literals
    m_semantic.insert(std::regex("^(true|false)"), TokenType_Literal, Type::Boolean);
    m_semantic.insert(std::regex(R"(^("[^"]*"))"), TokenType_Literal, Type::String);
    m_semantic.insert(std::regex("^(0|([1-9][0-9]*))(\\.[0-9]+)"), TokenType_Literal, Type::Double);
    m_semantic.insert(std::regex("^(0|([1-9][0-9]*))")           , TokenType_Literal, Type::Int16);

    // identifier
    m_semantic.insert(std::regex("^([a-zA-Z_]+[a-zA-Z0-9]*)"), TokenType_Identifier);

    // operators
    m_semantic.insert("operator", TokenType_KeywordOperator); // 3 chars
    m_semantic.insert(std::regex("^(<=>)"), TokenType_Operator); // 3 chars
    m_semantic.insert(std::regex("^([=\\|&]{2}|(<=)|(>=)|(=>)|(!=))"), TokenType_Operator); // 2 chars
    m_semantic.insert(std::regex("^[/+\\-*!=<>]"), TokenType_Operator); // single char

    add( new Operator("-"  , Operator_t::Unary, 5)); // --------- unary (sorted by precedence)
    add( new Operator("!"  , Operator_t::Unary, 5));

    add( new Operator("/"  , Operator_t::Binary, 20)); // ------- binary (sorted by precedence)
    add( new Operator("*"  , Operator_t::Binary, 20));
    add( new Operator("+"  , Operator_t::Binary, 10));
    add( new Operator("-"  , Operator_t::Binary, 10));
    add( new Operator("||" , Operator_t::Binary, 10));
    add( new Operator("&&" , Operator_t::Binary, 10));
    add( new Operator(">=" , Operator_t::Binary, 10));
    add( new Operator("<=" , Operator_t::Binary, 10));
    add( new Operator("=>" , Operator_t::Binary, 10));
    add( new Operator("==" , Operator_t::Binary, 10));
    add( new Operator("<=>", Operator_t::Binary, 10));
    add( new Operator("!=" , Operator_t::Binary, 10));
    add( new Operator(">"  , Operator_t::Binary, 10));
    add( new Operator("<"  , Operator_t::Binary, 10));
    add( new Operator("="  , Operator_t::Binary, 0));

    // operator implementations
    BIND_OPERATOR_T(api_add, "+", double(double, i16_t))
    BIND_OPERATOR_T(api_add, "+", double(double, double))
    BIND_OPERATOR_T(api_add, "+", i16_t(i16_t, i16_t))
    BIND_OPERATOR_T(api_add, "+", i16_t(i16_t, double))

    BIND_OPERATOR_T(api_concat, "+", string(string, string))
    BIND_OPERATOR_T(api_concat, "+", string(string, double))
    BIND_OPERATOR_T(api_concat, "+", string(string, i16_t))
    BIND_OPERATOR_T(api_concat, "+", string(string, bool))

    BIND_OPERATOR(api_or, "||")
    BIND_OPERATOR(api_and, "&&")

    BIND_OPERATOR_T(api_invert_sign, "-", double(double))
    BIND_OPERATOR_T(api_invert_sign, "-", i16_t(i16_t))

    BIND_OPERATOR_T(api_minus, "-", double(double, double))
    BIND_OPERATOR_T(api_minus, "-", double(double, i16_t))
    BIND_OPERATOR_T(api_minus, "-", i16_t(i16_t, i16_t))
    BIND_OPERATOR_T(api_minus, "-", i16_t(i16_t, double))

    BIND_OPERATOR_T(api_divide, "/", double(double, double))
    BIND_OPERATOR_T(api_divide, "/", double(double, i16_t))
    BIND_OPERATOR_T(api_divide, "/", i16_t(i16_t, i16_t))
    BIND_OPERATOR_T(api_divide, "/", i16_t(i16_t, double))

    BIND_OPERATOR_T(api_multiply, "*", double(double, double))
    BIND_OPERATOR_T(api_multiply, "*", double(double, i16_t))
    BIND_OPERATOR_T(api_multiply, "*", i16_t(i16_t, i16_t))
    BIND_OPERATOR_T(api_multiply, "*", i16_t(i16_t, double))

    BIND_OPERATOR_T(api_minus, "-", double(double, double))
    BIND_OPERATOR_T(api_minus, "-", double(double, i16_t))
    BIND_OPERATOR_T(api_minus, "-", i16_t(i16_t, i16_t))
    BIND_OPERATOR_T(api_minus, "-", i16_t(i16_t, double))

    BIND_OPERATOR_T(api_greater_or_eq, ">=", bool(double, double))
    BIND_OPERATOR_T(api_greater_or_eq, ">=", bool(double, i16_t))
    BIND_OPERATOR_T(api_greater_or_eq, ">=", bool(i16_t, double))
    BIND_OPERATOR_T(api_greater_or_eq, ">=", bool(i16_t, i16_t))

    BIND_OPERATOR_T(api_lower_or_eq, "<=", bool(double, i16_t))
    BIND_OPERATOR_T(api_lower_or_eq, "<=", bool(double, double))
    BIND_OPERATOR_T(api_lower_or_eq, "<=", bool(i16_t, i16_t))
    BIND_OPERATOR_T(api_lower_or_eq, "<=", bool(i16_t, double))

    BIND_OPERATOR(api_not, "!")

    BIND_OPERATOR_T(api_assign, "=", string(string & , string))
    BIND_OPERATOR_T(api_assign, "=", bool(bool & , bool))
    BIND_OPERATOR_T(api_assign, "=", double(double & , i16_t))
    BIND_OPERATOR_T(api_assign, "=", double(double & , double))
    BIND_OPERATOR_T(api_assign, "=", i16_t(i16_t & , i16_t))
    BIND_OPERATOR_T(api_assign, "=", i16_t(i16_t & , double))

    BIND_OPERATOR(api_implies, "=>")

    BIND_OPERATOR_T(api_equals, "==", bool(i16_t, i16_t))
    BIND_OPERATOR_T(api_equals, "==", bool(double, double))
    BIND_OPERATOR_T(api_equals, "==", bool(string, string))

    BIND_OPERATOR_T(api_equals, "<=>", bool(bool, bool))

    BIND_OPERATOR_T(api_not_equals, "!=", bool(bool, bool))
    BIND_OPERATOR_T(api_not_equals, "!=", bool(i16_t, i16_t))
    BIND_OPERATOR_T(api_not_equals, "!=", bool(double, double))
    BIND_OPERATOR_T(api_not_equals, "!=", bool(string, string))

    BIND_OPERATOR_T(api_greater, ">", bool(double, double))
    BIND_OPERATOR_T(api_greater, ">", bool(i16_t, i16_t))

    BIND_OPERATOR_T(api_lower, "<", bool(double, double))
    BIND_OPERATOR_T(api_lower, "<", bool(i16_t, i16_t))

    // functions

    BIND_FUNCTION_T(api_return, bool(bool))
    BIND_FUNCTION_T(api_return, i16_t(i16_t))
    BIND_FUNCTION_T(api_return, double(double))
    BIND_FUNCTION_T(api_return, string(string))

    BIND_FUNCTION(api_sin)
    BIND_FUNCTION(api_cos)
    BIND_FUNCTION_T(api_add, i16_t(i16_t, i16_t))
    BIND_FUNCTION_T(api_minus, i16_t(i16_t, i16_t))
    BIND_FUNCTION_T(api_multiply, i16_t(i16_t, i16_t))
    BIND_FUNCTION_T(api_add, double(double, double))
    BIND_FUNCTION_T(api_minus, double(double, double))
    BIND_FUNCTION_T(api_multiply, double(double, double))
    BIND_FUNCTION_T(api_sqrt, double(double))
    BIND_FUNCTION_T(api_sqrt, i16_t(i16_t))
    BIND_FUNCTION(api_not)
    BIND_FUNCTION(api_or)
    BIND_FUNCTION(api_and)
    BIND_FUNCTION(api_xor)
    BIND_FUNCTION(api_to_bool)
    BIND_FUNCTION(api_mod)
    BIND_FUNCTION_T(api_pow, i16_t(i16_t, i16_t))
    BIND_FUNCTION_T(api_pow, double(double, double))
    BIND_FUNCTION(api_secondDegreePolynomial)
    BIND_FUNCTION(api_DNAtoProtein)
    BIND_FUNCTION_T(api_to_string, string(bool))
    BIND_FUNCTION_T(api_to_string, string(double))
    BIND_FUNCTION_T(api_to_string, string(i16_t))
    BIND_FUNCTION_T(api_to_string, string(string))

    BIND_FUNCTION_T(api_print, string(bool))
    BIND_FUNCTION_T(api_print, string(double))
    BIND_FUNCTION_T(api_print, string(i16_t))
    BIND_FUNCTION_T(api_print, string(string))
}

void LanguageNodable::sanitize_function_identifier( std::string& _identifier ) const
{
    _identifier = regex_replace(_identifier, std::regex("^api_"), "");
}

void LanguageNodable::sanitize_operator_fct_identifier( std::string& _identifier ) const
{
    _identifier.insert(0, "operator");
}
