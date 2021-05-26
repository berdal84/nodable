#include <nodable/NodableLanguage.h>

#include <ctime>
#include <cmath>
#include <IconFontCppHeaders/IconsFontAwesome5.h>

#include <nodable/GraphNode.h>
#include <nodable/Member.h>
#include <nodable/NodableParser.h>
#include <nodable/NodableSerializer.h>
#include <nodable/Node.h>
#include <nodable/VariableNode.h>

using namespace Nodable;

NodableLanguage::NodableLanguage()
    :
    Language("Nodable", new NodableParser(this), new NodableSerializer(this))
{
    /*
     *  Configure the Semantic.
     *
     *  The order of insertion is important. First inserted will be taken in priority by Parser.
     */

    // comments
    semantic.insert(std::regex("^(//(.+?)$)"), TokenType_Ignore); // Single line
    semantic.insert(std::regex("^(/\\*(.+?)\\*/)"), TokenType_Ignore); // Multi line

    // conditionnal structures
    semantic.insert("if", TokenType_KeywordIf);
    semantic.insert("else", TokenType_KeywordElse);

    // punctuation
    semantic.insert(std::regex("^[ \t]"), TokenType_Ignore);
    semantic.insert("{", TokenType_BeginScope);
    semantic.insert("}", TokenType_EndScope);
    semantic.insert("(", TokenType_OpenBracket);
    semantic.insert(")", TokenType_CloseBracket);
    semantic.insert(",", TokenType_Separator);
    semantic.insert(" ", TokenType_Space);
    semantic.insert(";", TokenType_EndOfInstruction);
    semantic.insert("\n", TokenType_EndOfLine);

    // types
    semantic.insert("bool", TokenType_BooleanType);
    semantic.insert("string", TokenType_StringType);
    semantic.insert("double", TokenType_DoubleType);
    semantic.insert("any", TokenType_AnyType);

    // values
    semantic.insert(std::regex("^(true|false)"), TokenType_Boolean);
    semantic.insert(std::regex("^(\".*\")"), TokenType_String);
    semantic.insert(std::regex("^([a-zA-Z_]+[a-zA-Z0-9]*)"), TokenType_Identifier);
    semantic.insert(std::regex("^(0|([1-9][0-9]*))(\\.[0-9]+)?"), TokenType_Double);

    // operators
    semantic.insert("operator", TokenType_KeywordOperator); // 3 chars
    semantic.insert(std::regex("^(<=>)"), TokenType_Operator); // 3 chars
    semantic.insert(std::regex("^([=\\|&]{2}|(<=)|(>=)|(=>))"), TokenType_Operator); // 2 chars
    semantic.insert(std::regex("^[/+\\-*!=<>]"), TokenType_Operator); // single char

    // type correspondence
    semantic.insert(Type_Boolean, TokenType_BooleanType);
    semantic.insert(Type_Double, TokenType_DoubleType);
    semantic.insert(Type_String, TokenType_StringType);
    semantic.insert(Type_Any, TokenType_AnyType);


    /*
     * Create a minimal set of functions/operators
     */

    // To easily declare types
    auto Double = TokenType_DoubleType;
    auto Bool   = TokenType_BooleanType;
    auto Str    = TokenType_StringType;

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
    FCT_BEGIN(Str, "to_string", Double)
            RETURN( (std::string)ARG(0))
    FCT_END

    // string string(boolean)
    FCT_BEGIN(Str, "to_string", Bool)
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
            _args[0]->getInput()->set(ARG(1)); // TODO: implement references
		RETURN((double)ARG(1))
	OPERATOR_END

    // string operator=(string, string)
    BINARY_OP_BEGIN(Str, "=", Str, Str, 0u, ICON_FA_EQUALS " Assign")
            _args[0]->getInput()->set(ARG(1)); // TODO: implement references
            RETURN((std::string)ARG(1))
    OPERATOR_END

	// bool operator=(bool, bool)
	BINARY_OP_BEGIN(Bool, "=", Bool, Bool, 0u, ICON_FA_EQUALS " Assign")
            _args[0]->getInput()->set(ARG(1)); // // TODO: implement references
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
