#include "NodableLanguage.h"
#include "Member.h"
#include <ctime>
#include "IconsFontAwesome5.h"
#include <cmath>
#include "Node.h"
#include "VariableNode.h"
#include "GraphNode.h"

using namespace Nodable;

NodableLanguage::NodableLanguage(): Language("Nodable")
{
    /*
     *  Configure the Semantic.
     *
     *  The order of insertion is important. First inserted will be taken in priority by Parser.
     */

    // punctuation
    semantic.insert_RegexToTokenType(std::regex("^[ \t]"), TokenType::Ignore);
    semantic.insert_StringToTokenType("(", TokenType::OpenBracket);
    semantic.insert_StringToTokenType(")", TokenType::CloseBracket);
    semantic.insert_StringToTokenType(",", TokenType::Separator);
    semantic.insert_StringToTokenType(" ", TokenType::Space);
    semantic.insert_StringToTokenType(";", TokenType::EndOfInstruction);
    semantic.insert_StringToTokenType("\n", TokenType::EndOfLine);


    // values
    semantic.insert_RegexToTokenType(std::regex("^(true|false)"), TokenType::Boolean);
    semantic.insert_RegexToTokenType(std::regex("^(\"[a-zA-Z0-9 ]+\")"), TokenType::String);
    semantic.insert_RegexToTokenType(std::regex("^([a-zA-Z_]+)"), TokenType::Symbol);
    semantic.insert_RegexToTokenType(std::regex("^(0|([1-9][0-9]*))(\\.[0-9]+)?"), TokenType::Double);

    // types
    semantic.insert_StringToTokenType("bool", TokenType::BooleanType);
    semantic.insert_StringToTokenType("string", TokenType::StringType);
    semantic.insert_StringToTokenType("number", TokenType::DoubleType);
    semantic.insert_StringToTokenType("any", TokenType::AnyType);

    // operators
    semantic.insert_StringToTokenType("operator", TokenType::KeywordOperator); // 3 chars
    semantic.insert_RegexToTokenType(std::regex("^(<=>)"),TokenType::Operator); // 3 chars
    semantic.insert_RegexToTokenType(std::regex("^([=\\|&]{2}|(<=)|(>=)|(=>))"),TokenType::Operator); // 2 chars
    semantic.insert_RegexToTokenType(std::regex("^[/+\\-*!=<>]"),TokenType::Operator); // single char

    // comments
    semantic.insert_RegexToTokenType(std::regex("^(//(.+?)$)"),TokenType::Ignore); // Single line
    semantic.insert_RegexToTokenType(std::regex("^(/\\*(.+?)\\*/)"),TokenType::Ignore); // Multi line

    // type correspondence
    semantic.insert_TypeToTokenType(Type::Boolean, TokenType::BooleanType );
    semantic.insert_TypeToTokenType(Type::Double, TokenType::DoubleType );
    semantic.insert_TypeToTokenType(Type::String, TokenType::StringType );
    semantic.insert_TypeToTokenType(Type::Any, TokenType::AnyType );

    // conditionnal structures
    semantic.insert_StringToTokenType("if", TokenType::KeywordIf);
    semantic.insert_StringToTokenType("else", TokenType::KeywordElse);

    /*
     * Create a minimal set of functions/operators
     */

    // To easily declare types
    auto Double = TokenType::DoubleType;
    auto Bool   = TokenType::BooleanType;
    auto Str    = TokenType::StringType;

	// returnNumber(number)
	FCT_BEGIN(Double, "returnNumber", Double)
		RETURN( (double)ARG(0) )
	FCT_END

	// sin(number)
	FCT_BEGIN(Double, "sin", Double)
		RETURN( sin((double)ARG(0)) )
	FCT_END

	// cos(number)
	FCT_BEGIN(Double, "cos", Double)
		RETURN( cos((double)ARG(0)) )
	FCT_END

	// add(number)
	FCT_BEGIN(Double, "add", Double, Double)
		RETURN((double)ARG(0) + (double)ARG(1))
	FCT_END

	// minus(number)
	FCT_BEGIN(Double, "minus", Double, Double)
		RETURN( (double)ARG(0) - (double)ARG(1))
	FCT_END

	// mult(number)
	FCT_BEGIN(Double, "mult", Double, Double)
		RETURN( (double)ARG(0) * (double)ARG(1))
	FCT_END

	// sqrt(number)
	FCT_BEGIN(Double, "sqrt", Double)
		RETURN( sqrt((double)ARG(0)) )
	FCT_END

	// not(boolean)
	FCT_BEGIN(Bool, "not", Bool)
		RETURN( !(bool)ARG(0) )
	FCT_END

	// or(boolean, boolean)
	FCT_BEGIN(Bool, "or", Bool, Bool)
		RETURN( (bool)ARG(0) || ARG(1))
	FCT_END
	
	// and(boolean, boolean)
	FCT_BEGIN(Bool, "and", Bool, Bool)
		RETURN( (bool)ARG(0) && ARG(1))
	FCT_END

	// xor(boolean, boolean)
	FCT_BEGIN(Bool, "xor", Bool, Bool)
		RETURN(
		( (bool)ARG(0) && !(bool)ARG(1)) ||
		(!(bool)ARG(0) &&  (bool)ARG(1)) )
	FCT_END
	
	// boolean bool(number)
	FCT_BEGIN(Bool, "bool", Double)
		RETURN( (bool)ARG(0))
	FCT_END

    // string string(number)
    FCT_BEGIN(Str, "string", Double)
            RETURN( (std::string)ARG(0))
    FCT_END

    // string string(boolean)
    FCT_BEGIN(Str, "string", Bool)
        RETURN(ARG(0) ? "true" : "false" );
    FCT_END

	// mod(number, number)
	FCT_BEGIN(Double, "mod", Double, Double)
		RETURN( std::fmod((double)ARG(0), (double)ARG(1)) );
	FCT_END

	// pow(number)
	FCT_BEGIN(Double, "pow", Double, Double)
		RETURN( pow((double)ARG(0), (double)ARG(1)) )
	FCT_END
	
	// secondDegreePolynomial(a: number, x: number, b:number, y:number, c:number)
	FCT_BEGIN(Double, "secondDegreePolynomial", Double, Double, Double, Double, Double)
		const auto value = 
			(double)ARG(0) * pow((double)ARG(1), 2) * +  // ax� +
			(double)ARG(2) * (double)ARG(3) +            // by +
			(double)ARG(4);                              // c
	RETURN(value)
	FCT_END

	// DNAtoProtein(string)
	FCT_BEGIN(Str, "DNAtoProtein", Str)
		auto baseChain = (std::string)ARG(0);
		std::string protein = "";

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

		RETURN( protein )
	FCT_END

	// operator+(number, number)
	BINARY_OP_BEGIN(Double, "+", Double, Double, 10u, ICON_FA_PLUS " Add")
		RETURN( (double)ARG(0) + (double)ARG(1))
	OPERATOR_END

	// operator+(number, number)
	BINARY_OP_BEGIN(Str, "+", Str, Str, 10u, "Concat.")
	RETURN((std::string)ARG(0) + (std::string)ARG(1))
	OPERATOR_END

	// operator+(number, number)
	BINARY_OP_BEGIN(Str, "+", Str, Double, 10u, "Concat.")
	RETURN((std::string)ARG(0) + (std::string)ARG(1))
	OPERATOR_END

	// bool operator||(bool, bool)
	BINARY_OP_BEGIN(Bool, "||", Bool, Bool, 10u, "Logical Or")
	RETURN((bool)ARG(0) || (bool)ARG(1))
	OPERATOR_END

	// bool operator&&(bool, bool)
	BINARY_OP_BEGIN(Bool, "&&", Bool, Bool, 10u, "Logical And")
	RETURN((bool)ARG(0) && (bool)ARG(1))
	OPERATOR_END

	// operator-(number, number)	
	BINARY_OP_BEGIN(Double, "-", Double, Double, 10u, ICON_FA_MINUS " Subtract")
		RETURN( (double)ARG(0) - (double)ARG(1) )
	OPERATOR_END
	
	// operator/(number, number)
	BINARY_OP_BEGIN(Double, "/", Double, Double, 20u, ICON_FA_DIVIDE " Divide");
		RETURN( (double)ARG(0) / (double)ARG(1) )
	OPERATOR_END
	

	// operator*(number, number)
	BINARY_OP_BEGIN(Double, "*", Double, Double, 20u, ICON_FA_TIMES " Multiply")
		RETURN( (double)ARG(0) * (double)ARG(1) )
	OPERATOR_END

	// operator!(boolean)
	UNARY_OP_BEGIN(Bool, "!", Bool, 5u, "! not")
		RETURN( !(bool)ARG(0) )
	OPERATOR_END

	// operator-(number)
	UNARY_OP_BEGIN(Double, "-", Double, 5u, ICON_FA_MINUS " Minus")
		RETURN( -(double)ARG(0) )
	OPERATOR_END

	// number operator=(number, number)
	BINARY_OP_BEGIN(Double, "=", Double, Double, 0u, ICON_FA_EQUALS " Assign")
	    _args[0]->getInputMember()->set(ARG(1)); // TODO: find a better mecanism to declare out params
		RETURN((double)ARG(1))
	OPERATOR_END

    // string operator=(string, string)
    BINARY_OP_BEGIN(Str, "=", Str, Str, 0u, ICON_FA_EQUALS " Assign")
            _args[0]->getInputMember()->set(ARG(1)); // TODO: find a better mecanism to declare out params
            RETURN((std::string)ARG(1))
    OPERATOR_END

	// bool operator=(bool, bool)
	BINARY_OP_BEGIN(Bool, "=", Bool, Bool, 0u, ICON_FA_EQUALS " Assign")
            _args[0]->getInputMember()->set(ARG(1)); // TODO: find a better mecanism to declare out params
			RETURN((bool)ARG(1))
	OPERATOR_END

	// bool operator=>(bool, bool)
	BINARY_OP_BEGIN(Bool, "=>", Bool, Bool, 10u, "=> Implies")
		RETURN(!(bool)ARG(0) || (bool)ARG(1) )
	OPERATOR_END

	// operator>=(double, double)
	BINARY_OP_BEGIN(Bool, ">=", Double, Double, 10u, ">= Greater or equal")
		RETURN((double)ARG(0) >= (double)ARG(1))
	OPERATOR_END

	// operator<=(double, double)
	BINARY_OP_BEGIN(Bool, "<=", Double, Double, 10u, "<= Less or equal")
		RETURN((double)ARG(0) <= (double)ARG(1))
	OPERATOR_END

	// operator==(double, double)
	BINARY_OP_BEGIN(Bool, "==", Double, Double, 10u, "== Equals")
		RETURN((double)ARG(0) == (double)ARG(1))
	OPERATOR_END

	// operator<=>(bool, bool)
	BINARY_OP_BEGIN(Bool, "<=>", Bool, Bool, 10u, "<=> Equivalent")
	RETURN((double)ARG(0) == (double)ARG(1))
	OPERATOR_END

	// operator>(bool, bool)
	BINARY_OP_BEGIN(Bool, ">", Double, Double, 10u, "> Greater")
		RETURN((double)ARG(0) > (double)ARG(1))
	OPERATOR_END

	// operator<(bool, bool)
	BINARY_OP_BEGIN(Bool, "<", Double, Double,10u, "< Less")
		RETURN((double)ARG(0) < (double)ARG(1))
	OPERATOR_END
}
