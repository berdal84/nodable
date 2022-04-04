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
    POLYOPER(api_add, "+", double(double, i16_t))
    POLYOPER(api_add, "+", double(double, double))
    POLYOPER(api_add, "+", i16_t(i16_t, i16_t))
    POLYOPER(api_add, "+", i16_t(i16_t, double))

    POLYOPER(api_concat, "+", string(string, string))
    POLYOPER(api_concat, "+", string(string, double))
    POLYOPER(api_concat, "+", string(string, i16_t))
    POLYOPER(api_concat, "+", string(string, bool))

    OPER(api_or, "||")
    OPER(api_and, "&&")

    POLYOPER(api_invert_sign, "-", double(double))
    POLYOPER(api_invert_sign, "-", i16_t(i16_t))

    POLYOPER(api_minus, "-", double(double, double))
    POLYOPER(api_minus, "-", double(double, i16_t))
    POLYOPER(api_minus, "-", i16_t(i16_t, i16_t))
    POLYOPER(api_minus, "-", i16_t(i16_t, double))

    POLYOPER(api_divide, "/", double(double, double))
    POLYOPER(api_divide, "/", double(double, i16_t))
    POLYOPER(api_divide, "/", i16_t(i16_t, i16_t))
    POLYOPER(api_divide, "/", i16_t(i16_t, double))

    POLYOPER(api_multiply, "*", double(double, double))
    POLYOPER(api_multiply, "*", double(double, i16_t))
    POLYOPER(api_multiply, "*", i16_t(i16_t, i16_t))
    POLYOPER(api_multiply, "*", i16_t(i16_t, double))

    POLYOPER(api_minus, "-", double(double, double))
    POLYOPER(api_minus, "-", double(double, i16_t))
    POLYOPER(api_minus, "-", i16_t(i16_t, i16_t))
    POLYOPER(api_minus, "-", i16_t(i16_t, double))

    POLYOPER(api_greater_or_eq, ">=", bool(double, double))
    POLYOPER(api_greater_or_eq, ">=", bool(double, i16_t))
    POLYOPER(api_greater_or_eq, ">=", bool(i16_t, double))
    POLYOPER(api_greater_or_eq, ">=", bool(i16_t, i16_t))

    POLYOPER(api_lower_or_eq, "<=", bool(double, i16_t))
    POLYOPER(api_lower_or_eq, "<=", bool(double, double))
    POLYOPER(api_lower_or_eq, "<=", bool(i16_t, i16_t))
    POLYOPER(api_lower_or_eq, "<=", bool(i16_t, double))

    OPER(api_not, "!")

    POLYOPER(api_assign, "=", string(string & , string))
    POLYOPER(api_assign, "=", bool(bool & , bool))
    POLYOPER(api_assign, "=", double(double & , i16_t))
    POLYOPER(api_assign, "=", double(double & , double))
    POLYOPER(api_assign, "=", i16_t(i16_t & , i16_t))
    POLYOPER(api_assign, "=", i16_t(i16_t & , double))

    OPER(api_implies, "=>")

    POLYOPER(api_equals, "==", bool(i16_t, i16_t))
    POLYOPER(api_equals, "==", bool(double, double))
    POLYOPER(api_equals, "==", bool(string, string))

    POLYOPER(api_equals, "<=>", bool(bool, bool))

    POLYOPER(api_not_equals, "!=", bool(bool, bool))
    POLYOPER(api_not_equals, "!=", bool(i16_t, i16_t))
    POLYOPER(api_not_equals, "!=", bool(double, double))
    POLYOPER(api_not_equals, "!=", bool(string, string))

    POLYOPER(api_greater, ">", bool(double, double))
    POLYOPER(api_greater, ">", bool(i16_t, i16_t))

    POLYOPER(api_lower, "<", bool(double, double))
    POLYOPER(api_lower, "<", bool(i16_t, i16_t))

    // functions

    POLYFUNC(api_return, bool(bool))
    POLYFUNC(api_return, i16_t(i16_t))
    POLYFUNC(api_return, double(double))
    POLYFUNC(api_return, string(string))

    FUNC(api_sin)
    FUNC(api_cos)
    POLYFUNC(api_add, i16_t(i16_t, i16_t))
    POLYFUNC(api_minus, i16_t(i16_t, i16_t))
    POLYFUNC(api_multiply, i16_t(i16_t, i16_t))
    POLYFUNC(api_add, double(double, double))
    POLYFUNC(api_minus, double(double, double))
    POLYFUNC(api_multiply, double(double, double))
    POLYFUNC(api_sqrt, double(double))
    POLYFUNC(api_sqrt, double(i16_t))
    FUNC(api_not)
    FUNC(api_or)
    FUNC(api_and)
    FUNC(api_xor)
    FUNC(api_to_bool)
    FUNC(api_mod)
    POLYFUNC(api_pow, i16_t(i16_t, i16_t))
    POLYFUNC(api_pow, double(double, double))
    FUNC(api_secondDegreePolynomial)
    FUNC(api_DNAtoProtein)
    POLYFUNC(api_to_string, string(bool))
    POLYFUNC(api_to_string, string(double))
    POLYFUNC(api_to_string, string(i16_t))
    POLYFUNC(api_to_string, string(string))

    POLYFUNC(api_print, string(bool))
    POLYFUNC(api_print, string(double))
    POLYFUNC(api_print, string(i16_t))
    POLYFUNC(api_print, string(string))
}

void LanguageNodable::sanitize_function_identifier( std::string& _identifier ) const
{
    _identifier = regex_replace(_identifier, std::regex("^api_"), "");
}

void LanguageNodable::sanitize_operator_fct_identifier( std::string& _identifier ) const
{
    _identifier.insert(0, "operator");
}
