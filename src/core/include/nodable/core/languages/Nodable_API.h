#include <nodable/core/languages/Nodable.h>

#include <ctime>
#include <cmath>
#include <IconFontCppHeaders/IconsFontAwesome5.h>

#include <nodable/core/GraphNode.h>
#include <nodable/core/Member.h>
#include <nodable/core/Node.h>
#include <nodable/core/VariableNode.h>
#include <nodable/core/String.h>
#include <nodable/core/System.h>

using namespace Nodable;
using namespace Nodable::R;

/*
 * Define functions/operators to be wrapped in nodable classes in order to be invoked at runtime.
 */

// FUNCTIONS

// boolean operations
bool api_and(bool a, bool b) { return a && b; }
template<typename T, typename U> bool api_greater(T a, U b) { return a > b; }
template<typename T, typename U> bool api_greater_or_eq(T a, U b) { return a >= b; }
bool api_implies(bool a, bool b) { return !a || b; }
template<typename T, typename U> bool api_lower(T a, U b) { return a < b; }
template<typename T, typename U> bool api_lower_or_eq(T a, U b) { return a <= b; }
bool api_not(bool b) { return !b; }
bool api_or(bool a, bool b) { return a || b; }
bool api_to_bool(double n) { return n == 0.0; }
bool api_xor(bool a, bool b){ return a ^ b; }
template<typename T, typename U>  T api_add(T a, U b){ return a + b; }
double api_cos(double n) { return cos(n); }
template<typename T, typename U>  T api_divide(T a, U b) { if ( b == 0 ) throw std::runtime_error("division by zero !"); return a / b;}
double api_invert_sign(double a) { return -a; }
template<typename T> T api_invert_sign(T a) { return -a; }
template<typename T, typename U>  T  api_minus(T a, U b){ return a - b; }
template<typename T, typename U>  T  api_multiply(T a, U b) { return a * b; }
double api_sin(double n) { return sin(n); }
double api_mod(double a, double b) { return fmod(a, b); }
template<typename T>  T api_pow(T a, T b) { return pow(a, b); }
double api_secondDegreePolynomial(double a, double x, double b, double y, double c) { return a * x * x + b * y + c;}
template<typename T, typename U> T api_sqrt(U n) { return sqrt(n); }
std::string api_to_string(i16_t i) { return std::to_string(i); }
std::string api_to_string(bool b) { return b ? "true" : "false"; }
std::string api_to_string(double n) { return String::fmt_double(n); }
std::string api_to_string(std::string s) { return s; }
template<typename T> T api_return(T value) { return value; }
template<typename T, typename U> T api_assign(T& a, U b) { return a = b; }
template<typename T> bool api_equals(T a, T b) { return a == b; }
template<typename T> bool api_not_equals(T a, T b) { return a != b; }
template<typename T> std::string api_concat(std::string left, T right) { return  left +
            Nodable::String::fmt_double(right); }
template<> std::string api_concat(std::string left, std::string right) { return  left + right; }

template<typename T>
std::string api_print(T _value)
{
    std::string result = api_to_string(_value);
    printf("print: %s\n", result.c_str());
    return result;
}

std::string api_DNAtoProtein(std::string baseChain)
{
    std::string protein;

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