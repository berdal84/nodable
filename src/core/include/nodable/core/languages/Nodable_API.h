#include <nodable/core/languages/Nodable.h>

#include <ctime>
#include <cmath>
#include <IconFontCppHeaders/IconsFontAwesome5.h>

#include <nodable/core/GraphNode.h>
#include <nodable/core/Member.h>
#include <nodable/core/Node.h>
#include <nodable/core/VariableNode.h>
#include <nodable/core/Format.h>
#include <nodable/core/System.h>

using namespace Nodable;
using namespace Nodable::R;

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
    if ( b == 0.0 ) throw std::runtime_error("division by zero !");
    return a / b;
}

double api_sqrt(double n)
{
    return sqrt(n);
}

bool api_not(bool b)
{
    return !b;
}

template<typename T>
T api_assign(T& a, T b)
{
    return a = b;
}

bool api_implies(bool a, bool b)
{
    return !a || b;
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

template<typename T>
std::string api_concat(std::string left, T right)
{
    return  left + Nodable::Format::fmt_no_trail(right);
}

template<>
std::string api_concat(std::string left, std::string right)
{
    return  left + right;
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


template<typename T>
bool api_equals(T a, T b)
{
    return a == b;
}

template<typename T>
bool api_not_equals(T a, T b)
{
    return a != b;
}

bool api_greater(double a, double b)
{
    return a > b;
}

bool api_lower(double a, double b)
{
    return a < b;
}

std::string api_to_string(double n) { return Format::fmt_no_trail(n); }
std::string api_to_string(bool b) { return b ? "true" : "false"; }
std::string api_to_string(std::string s) { return s; }

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