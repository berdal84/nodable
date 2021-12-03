#include <nodable/NodableLanguage.h>

#include <ctime>
#include <cmath>
#include <IconFontCppHeaders/IconsFontAwesome5.h>

#include <nodable/GraphNode.h>
#include <nodable/Member.h>
#include <nodable/Node.h>
#include <nodable/VariableNode.h>

using namespace Nodable;

/*
 * Define functions/operators to be wrapped in nodable classes in order to be invoked at runtime.
 */

// FUNCTIONS

double api_returnNumber(double n)
{
    return n;
}

double api_sin(double n)
{
    return sin(n);
}

double api_cos(double n)
{
    return cos(n);
}

double api_add(double a, double b)
{
    return a + b;
}

double api_minus(double a, double b)
{
    return a - b;
}

double api_invert_sign(double a)
{
    return -a;
}

double api_multiply(double a, double b)
{
    return a * b;
}

double api_divide(double a, double b)
{
    return a / b;
}

double api_sqrt(double n)
{
    return sqrt(n);
}

double api_not(bool b)
{
    return !b;
}

double api_assign_d(double a, double b)
{
    return a = b;
}

bool api_assign_b(bool a, bool b)
{
    return a = b;
}

bool api_implies(bool a, bool b)
{
    return !a || b;
}

std::string api_assign_s(std::string a, std::string b)
{
    return a = b;
}

bool api_and(bool a, bool b)
{
    return a && b;
}

bool api_or(bool a, bool b)
{
    return a || b;
}

bool api_xor(bool a, bool b)
{
    return a ^ b;
}

bool api_to_bool(double n)
{
    return n == 0.0;
}

std::string api_concat(std::string left, std::string right)
{
    left = right;
    return  left;
}

double api_mod(double a, double b)
{
    return fmod(a, b);
}

double api_pow(double a, double b)
{
    return pow(a, b);
}

double api_secondDegreePolynomial(double a, double x, double b, double y, double c)
{
    return a * x * x + b * y + c;
}

bool api_greater_or_eq(double a, double b)
{
    return a >= b;
}

bool api_lower_or_eq(double a, double b)
{
    return a <= b;
}

bool api_equals(double a, double b)
{
    return a == b;
}

bool api_equivalent(bool a, bool b)
{
    return a == b;
}

bool api_greater(double a, double b)
{
    return a > b;
}

bool api_lower(double a, double b)
{
    return a < b;
}

std::string api_to_string(double n)
{
    return std::to_string(n);
}

std::string api_to_string(bool b)
{
    return b ? "true" : "false";
}

std::string api_DNAtoProtein(std::string baseChain)
{
    std::string protein = "";

    // todo: change this naive approach by 3 tests, one per base.
    std::map<std::string, char> table;
    {
        table["ATA"] = 'I'; // A__
        table["ATC"] = 'I';
        table["ATT"] = 'I';
        table["ATG"] = 'M'; // (aka Start)
        table["ACA"] = 'T';
        table["ACC"] = 'T';
        table["ACG"] = 'T';
        table["ACT"] = 'T';
        table["AAC"] = 'N';
        table["AAT"] = 'N';
        table["AAA"] = 'K';
        table["AAG"] = 'K';
        table["AGC"] = 'S';
        table["AGT"] = 'S';
        table["AGA"] = 'R';
        table["AGG"] = 'R';
        table["CTA"] = 'L'; // C__
        table["CTC"] = 'L';
        table["CTG"] = 'L';
        table["CTT"] = 'L';
        table["CCA"] = 'P';
        table["CCC"] = 'P';
        table["CCG"] = 'P';
        table["CCT"] = 'P';
        table["CAC"] = 'H';
        table["CAT"] = 'H';
        table["CAA"] = 'Q';
        table["CAG"] = 'Q';
        table["CGA"] = 'R';
        table["CGC"] = 'R';
        table["CGG"] = 'R';
        table["CGT"] = 'R';
        table["GTA"] = 'V'; // G__
        table["GTC"] = 'V';
        table["GTG"] = 'V';
        table["GTT"] = 'V';
        table["GCA"] = 'A';
        table["GCC"] = 'A';
        table["GCG"] = 'A';
        table["GCT"] = 'A';
        table["GAC"] = 'D';
        table["GAT"] = 'D';
        table["GAA"] = 'E';
        table["GAG"] = 'E';
        table["GGA"] = 'G';
        table["GGC"] = 'G';
        table["GGG"] = 'G';
        table["GGT"] = 'G';
        table["TCA"] = 'S'; // T__
        table["TCC"] = 'S';
        table["TCG"] = 'S';
        table["TCT"] = 'S';
        table["TTC"] = 'F';
        table["TTT"] = 'F';
        table["TTA"] = 'L';
        table["TTG"] = 'L';
        table["TAC"] = 'Y';
        table["TAT"] = 'Y';
        table["TAA"] = '_'; // (aka Stop)
        table["TAG"] = '_'; // (aka Stop)
        table["TGC"] = 'C';
        table["TGT"] = 'C';
        table["TGA"] = '_'; // (aka Stop)
        table["TGG"] = 'W';
    }

    for (size_t i = 0; i < baseChain.size() / 3; i++) {
        auto found = table.find(baseChain.substr(i, 3));
        if (found != table.end())
            protein += found->second;
    }
    return protein;
}

void NodableLanguage::sanitizeFunctionName( std::string& name ) const
{
    name = regex_replace(name, std::regex("^api_"), "");
}

NodableLanguage::NodableLanguage()
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
    semantic.insert("bool",   TokenType_KeywordBoolean, Type_Boolean); // types
    semantic.insert("string", TokenType_KeywordString,  Type_String);
    semantic.insert("double", TokenType_KeywordDouble,  Type_Double);
    semantic.insert("any",    TokenType_KeywordAny,     Type_Any);

    // punctuation
    semantic.insert("{", TokenType_BeginScope);
    semantic.insert("}", TokenType_EndScope);
    semantic.insert("(", TokenType_OpenBracket);
    semantic.insert(")", TokenType_CloseBracket);
    semantic.insert(",", TokenType_Separator);
    semantic.insert(";", TokenType_EndOfInstruction);
    semantic.insert("\n", TokenType_EndOfLine);

    // literals
    semantic.insert(std::regex("^(true|false)"), TokenType_Literal, Type_Boolean);
    semantic.insert(std::regex("^(\".*\")"), TokenType_Literal, Type_String);
    semantic.insert(std::regex("^(0|([1-9][0-9]*))(\\.[0-9]+)?"), TokenType_Literal, Type_Double);

    // identifier
    semantic.insert(std::regex("^([a-zA-Z_]+[a-zA-Z0-9]*)"), TokenType_Identifier);

    // operators
    semantic.insert("operator", TokenType_KeywordOperator); // 3 chars
    semantic.insert(std::regex("^(<=>)"), TokenType_Operator); // 3 chars
    semantic.insert(std::regex("^([=\\|&]{2}|(<=)|(>=)|(=>))"), TokenType_Operator); // 2 chars
    semantic.insert(std::regex("^[/+\\-*!=<>]"), TokenType_Operator); // single char

    /*
     * Wrap a minimal set of functions/operators
     */


    WRAP_FUNCTION(api_returnNumber)
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
    WRAP_POLYFUNCTION(api_to_string, std::string(bool))
    WRAP_POLYFUNCTION(api_to_string, std::string(double))
    WRAP_FUNCTION(api_mod)
    WRAP_FUNCTION(api_pow)
    WRAP_FUNCTION(api_secondDegreePolynomial)
	WRAP_FUNCTION(api_DNAtoProtein)

	WRAP_OPERATOR(api_add        , "+" , 10, ICON_FA_PLUS " Add")
    WRAP_OPERATOR(api_concat     , "+" , 10, "Concat.")
    WRAP_OPERATOR(api_concat     , "+" , 10, "Concat.")
    WRAP_OPERATOR(api_or         , "||", 10, "Logical Or")
    WRAP_OPERATOR(api_and        , "&&", 10, "Logical And")
    WRAP_OPERATOR(api_invert_sign, "-" , 10, ICON_FA_MINUS " Invert Sign")
    WRAP_OPERATOR(api_minus      , "-" , 10, ICON_FA_MINUS " Subtract")
    WRAP_OPERATOR(api_divide     , "/" , 20, ICON_FA_DIVIDE " Divide")
    WRAP_OPERATOR(api_multiply   , "*" , 20, ICON_FA_TIMES " Multiply")
    WRAP_OPERATOR(api_not        , "!" , 5 , "! not")
    WRAP_OPERATOR(api_minus      , "-" , 5 , ICON_FA_MINUS " Minus")
    WRAP_OPERATOR(api_assign_d   , "=" , 0, ICON_FA_EQUALS " Assign")
    WRAP_OPERATOR(api_assign_b   , "=" , 0, ICON_FA_EQUALS " Assign")
    WRAP_OPERATOR(api_assign_s   , "=" , 0, ICON_FA_EQUALS " Assign")
    WRAP_OPERATOR(api_implies    , "=>", 10, "=> Implies")
    WRAP_OPERATOR(api_greater_or_eq, ">=", 10u, ">= Greater or equal")
    WRAP_OPERATOR(api_lower_or_eq, "<=", 10u, "<= Less or equal")
    WRAP_OPERATOR(api_equals     , "==", 10u, "== Equals")
    WRAP_OPERATOR(api_equivalent , "<=>", 10, "<=> Equivalent")
    WRAP_OPERATOR(api_greater    ,">" , 10u, "> Greater")
    WRAP_OPERATOR(api_lower       , "<", 10u, "< Less")
}
