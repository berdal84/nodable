#include <nodable/core/languages/Nodable.h>

#include <nodable/core/Member.h>
#include <nodable/core/Format.h>
#include <nodable/core/System.h>

#include <nodable/core/languages/Nodable_API.h> // <----- contains all functions referenced below

using namespace Nodable;
using namespace Nodable::R;

void LanguageNodable::sanitizeFunctionName( std::string& name ) const
{
    name = regex_replace(name, std::regex("^api_"), "");
}

void LanguageNodable::sanitizeOperatorFunctionName( std::string& name ) const
{
    name.insert(0, "operator");
}

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
    semantic.insert(std::regex("^(//(.+?)$)"), TokenType_Ignore);      // Single line
    semantic.insert(std::regex("^(/\\*(.+?)\\*/)"), TokenType_Ignore); // Multi line
    semantic.insert("\t", TokenType_Ignore);
    semantic.insert(" ", TokenType_Ignore);

    // keywords
    semantic.insert("if", TokenType_KeywordIf);                        // conditional structures
    semantic.insert("else", TokenType_KeywordElse);
    semantic.insert("for", TokenType_KeywordFor);
    semantic.insert("bool", TokenType_KeywordBoolean, Type::Boolean); // types
    semantic.insert("string", TokenType_KeywordString, Type::String);
    semantic.insert("double", TokenType_KeywordDouble, Type::Double);

    // punctuation
    semantic.insert("{", TokenType_BeginScope);
    semantic.insert("}", TokenType_EndScope);
    semantic.insert("(", TokenType_OpenBracket);
    semantic.insert(")", TokenType_CloseBracket);
    semantic.insert(",", TokenType_Separator);
    semantic.insert(";", TokenType_EndOfInstruction);
    semantic.insert(std::string{System::k_end_of_line}, TokenType_EndOfLine);

    // literals
    semantic.insert(std::regex("^(true|false)"), TokenType_Literal, Type::Boolean);
    semantic.insert(std::regex(R"(^("[^"]*"))"), TokenType_Literal, Type::String);
    semantic.insert(std::regex("^(0|([1-9][0-9]*))(\\.[0-9]+)?"), TokenType_Literal, Type::Double);

    // identifier
    semantic.insert(std::regex("^([a-zA-Z_]+[a-zA-Z0-9]*)"), TokenType_Identifier);

    // operators
    semantic.insert("operator", TokenType_KeywordOperator); // 3 chars
    semantic.insert(std::regex("^(<=>)"), TokenType_Operator); // 3 chars
    semantic.insert(std::regex("^([=\\|&]{2}|(<=)|(>=)|(=>)|(!=))"), TokenType_Operator); // 2 chars
    semantic.insert(std::regex("^[/+\\-*!=<>]"), TokenType_Operator); // single char

    /*
     * Wrap a minimal set of functions/operators
     */

    using string = std::string;

    WRAP_POLYFUNC(api_return, bool(bool))
    WRAP_POLYFUNC(api_return, double(double))
    WRAP_POLYFUNC(api_return, string(string))
    WRAP_FUNCTION(api_sin)
    WRAP_FUNCTION(api_cos)
    WRAP_FUNCTION(api_add)
    WRAP_FUNCTION(api_minus)
    WRAP_FUNCTION(api_multiply)
    WRAP_FUNCTION(api_sqrt)
    WRAP_FUNCTION(api_not)
    WRAP_FUNCTION(api_or)
    WRAP_FUNCTION(api_and)
    WRAP_FUNCTION(api_xor)
    WRAP_FUNCTION(api_to_bool)
    WRAP_FUNCTION(api_mod)
    WRAP_FUNCTION(api_pow)
    WRAP_FUNCTION(api_secondDegreePolynomial)
	WRAP_FUNCTION(api_DNAtoProtein)

    WRAP_POLYFUNC(api_to_string, string(bool))
    WRAP_POLYFUNC(api_to_string, string(double))
    WRAP_POLYFUNC(api_to_string, string(string))
    WRAP_POLYFUNC(api_print,     string(bool))
    WRAP_POLYFUNC(api_print,     string(double))
    WRAP_POLYFUNC(api_print,     string(string))

	WRAP_OPERATOR(api_add        , "+" , 10, ICON_FA_PLUS " Add")
    WRAP_POLYOPER(api_concat     , "+" , 10, "Concat.", string(string, string))
    WRAP_POLYOPER(api_concat     , "+" , 10, "Concat.", string(string, double))
    WRAP_POLYOPER(api_concat     , "+" , 10, "Concat.", string(string, bool))
    WRAP_OPERATOR(api_or         , "||", 10, "Logical Or")
    WRAP_OPERATOR(api_and        , "&&", 10, "Logical And")
    WRAP_OPERATOR(api_invert_sign, "-" , 10, ICON_FA_MINUS " Invert Sign")
    WRAP_OPERATOR(api_minus      , "-" , 10, ICON_FA_MINUS " Subtract")
    WRAP_OPERATOR(api_divide     , "/" , 20, ICON_FA_DIVIDE " Divide")
    WRAP_OPERATOR(api_multiply   , "*" , 20, ICON_FA_TIMES " Multiply")
    WRAP_OPERATOR(api_not        , "!" , 5 , "! not")
    WRAP_OPERATOR(api_minus      , "-" , 5 , ICON_FA_MINUS " Minus")
    WRAP_POLYOPER(api_assign     , "=" , 0, ICON_FA_EQUALS " Assign", string(string&, string) )
    WRAP_POLYOPER(api_assign     , "=" , 0, ICON_FA_EQUALS " Assign", bool(bool&, bool) )
    WRAP_POLYOPER(api_assign     , "=" , 0, ICON_FA_EQUALS " Assign", double(double&, double) )
    WRAP_OPERATOR(api_implies    , "=>", 10, "=> Implies")
    WRAP_OPERATOR(api_greater_or_eq, ">=", 10, ">= Greater or equal")
    WRAP_OPERATOR(api_lower_or_eq, "<=", 10, "<= Less or equal")
    WRAP_POLYOPER(api_equals  , "==", 10, "== Equals" , bool(double, double) )
    WRAP_POLYOPER(api_equals  , "==", 10, "== Equals" , bool(string, string) )
    WRAP_POLYOPER(api_equals  , "<=>", 10, "<=> Equivalent" , bool(bool, bool) )
    WRAP_POLYOPER(api_not_equals  , "!=", 10, "!= Not equal" , bool(bool, bool) )
    WRAP_POLYOPER(api_not_equals  , "!=", 10, "!= Not equal" , bool(double, double) )
    WRAP_POLYOPER(api_not_equals  , "!=", 10, "!= Not equal" , bool(string, string) )
    WRAP_OPERATOR(api_greater ,">" , 10, "> Greater")
    WRAP_OPERATOR(api_lower   , "<", 10, "< Less")
}
