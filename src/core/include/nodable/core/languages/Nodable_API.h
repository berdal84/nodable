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
bool api_implies(bool a, bool b) { return !a || b; }
bool api_not(bool b) { return !b; }
bool api_or(bool a, bool b) { return a || b; }
bool api_to_bool(double n) { return n == 0.0; }
bool api_xor(bool a, bool b){ return a ^ b; }
double api_cos(double n) { return cos(n); }
double api_mod(double a, double b) { return fmod(a, b); }
double api_secondDegreePolynomial(double a, double x, double b, double y, double c) { return a * x * x + b * y + c;}
double api_sin(double n) { return sin(n); }
std::string api_to_string(bool b) { return b ? "true" : "false"; }
std::string api_to_string(double n) { return String::fmt_double(n); }
std::string api_to_string(i16_t i) { return std::to_string(i); }
std::string api_to_string(std::string s) { return s; }
template<typename T>
    std::string api_concat(std::string left, T right) { return  left + Nodable::String::fmt_double(right); }
template<>
    std::string api_concat(std::string left, std::string right) { return  left + right; }
template<typename T, typename U>
    T api_minus(T a, U b){ return a - b; }
template<typename T, typename U>
    T api_multiply(T a, U b) { return a * b; }
template<typename T, typename U>
    T api_add(T a, U b){ return a + b; }
template<typename T, typename U>
    T api_divide(T a, U b) { if ( b == 0 ) throw std::runtime_error("division by zero !"); return a / b;}
template<typename T, typename U>
    T api_assign(T& a, U b) { return a = b; }
template<typename T, typename U>
    T api_sqrt(U n) { return sqrt(n); }
template<typename T, typename U>
    bool api_greater(T a, U b) { return a > b; }
template<typename T, typename U>
    bool api_greater_or_eq(T a, U b) { return a >= b; }
template<typename T, typename U>
    bool api_lower(T a, U b) { return a < b; }
template<typename T, typename U>
    bool api_lower_or_eq(T a, U b) { return a <= b; }
template<typename T>
    T api_pow(T a, T b) { return pow(a, b); }
template<typename T>
    T api_minus(T a) { return -a; }
template<typename T>
    T api_return(T value) { return value; }
template<typename T>
    bool api_equals(T a, T b) { return a == b; }
template<typename T>
    bool api_not_equals(T a, T b) { return a != b; }
template<typename T>
    std::string api_print(T _value)
    {
        std::string result = api_to_string(_value);
        printf("print: %s\n", result.c_str());
        return result;
    }

std::string api_DNAtoProtein(std::string _base_str)
{
    std::string protein;

    // todo: change this naive approach by 3 tests, one per base.
    const std::map<const std::string, const char> table
    {
        // T__
        {"TCA", 'S'}, {"TCC", 'S'}, {"TCG", 'S'}, {"TCT", 'S'},
        {"TTC", 'F'}, {"TTT", 'F'}, {"TTA", 'L'}, {"TGA", '_'}, // (aka Stop)
        {"TAC", 'Y'}, {"TAT", 'Y'}, {"TGC", 'C'}, {"TAG", '_'}, // (aka Stop)
        {"TGT", 'C'}, {"TGG", 'W'}, {"TTG", 'L'}, {"TAA", '_'}, // (aka Stop)

        // A__
        {"ATA", 'I'}, {"ATC", 'I'}, {"ATT", 'I'}, {"ATG", 'M'}, // (aka Start)
        {"ACA", 'T'}, {"ACC", 'T'}, {"ACG", 'T'}, {"ACT", 'T'},
        {"AAC", 'N'}, {"AAT", 'N'}, {"AAA", 'K'}, {"AAG", 'K'},
        {"AGC", 'S'}, {"AGT", 'S'}, {"AGA", 'R'}, {"AGG", 'R'},

        // C__
        {"CTA", 'L'}, {"CTC", 'L'}, {"CTG", 'L'}, {"CTT", 'L'},
        {"CCA", 'P'}, {"CCC", 'P'}, {"CCG", 'P'}, {"CCT", 'P'},
        {"CAC", 'H'}, {"CAT", 'H'}, {"CAA", 'Q'}, {"CAG", 'Q'},
        {"CGA", 'R'}, {"CGC", 'R'}, {"CGG", 'R'}, {"CGT", 'R'},

        // G__
        {"GTA", 'V'}, {"GTC", 'V'}, {"GTG", 'V'}, {"GTT", 'V'},
        {"GCA", 'A'}, {"GCC", 'A'}, {"GCG", 'A'}, {"GCT", 'A'},
        {"GAC", 'D'}, {"GAT", 'D'}, {"GAA", 'E'}, {"GAG", 'E'},
        {"GGA", 'G'}, {"GGC", 'G'}, {"GGG", 'G'}, {"GGT", 'G'},
    };

    for (size_t i = 0; i < _base_str.size() / 3; i++)
    {
        std::string possibly_codon = _base_str.substr(i, 3);

        auto found = table.find( possibly_codon );

        if( found != table.cend() )
        {
            auto [key, translation] = *found;
            protein.push_back( translation );
        }
    }

    return protein;
}