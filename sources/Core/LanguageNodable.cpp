#include "LanguageNodable.h"
#include "Member.h"
#include <time.h>
#include "IconsFontAwesome5.h"

using namespace Nodable;

std::string LanguageNodable::serialize(
	const FunctionSignature&   _signature,
	std::vector<const Member*> _args) const
{
	std::string expr;
	expr.append(_signature.getIdentifier());
	expr.append("( ");

	for (auto it = _args.begin(); it != _args.end(); it++) {
		expr.append((*it)->getSourceExpression());

		if (*it != _args.back()) {
			expr.append(", ");
		}
	}

	expr.append(" )");
	return expr;

}

std::string LanguageNodable::serialize(const FunctionSignature& _signature) const {

	std::string result = _signature.getIdentifier() + serialize(TokenType::LBracket);
	auto args = _signature.getArgs();

	for (auto it = args.begin(); it != args.end(); it++) {

		if (it != args.begin()) {
			result.append(serialize(TokenType::Separator));
			result.append(serialize(TokenType::Space));
		}
		const auto argType = (*it).type;
		result.append( serialize(argType) );

	}

	result.append( serialize(TokenType::RBracket) );

	return result;

}

std::string LanguageNodable::serialize(const TokenType& _type) const {
	return tokenTypeToString.at(_type);
}

LanguageNodable::LanguageNodable(): Language("Nodable")
{
	// Prepare regex for some TokenType
	tokenTypeToRegex[TokenType::Str]    = std::regex("^\"[a-zA-Z0-9 ]+\"");
	tokenTypeToRegex[TokenType::Symbol] = std::regex("^[a-zA-Z_]+");
	tokenTypeToRegex[TokenType::Double] = std::regex("^(0|([1-9][0-9]*))(\\.[0-9]+)?");

	// Fill string to type map
	keywordToTokenType[" "]        = TokenType::Space;
	keywordToTokenType["("]        = TokenType::LBracket;
	keywordToTokenType[")"]        = TokenType::RBracket;
	keywordToTokenType[","]        = TokenType::Separator;
	keywordToTokenType["\t"]       = TokenType::Tab;
	keywordToTokenType["string"]   = TokenType::Str;
	keywordToTokenType["number"]   = TokenType::Double;
	keywordToTokenType["symbol"]   = TokenType::Symbol;
	keywordToTokenType["operator"] = TokenType::Operator;
	keywordToTokenType["bool"]     = TokenType::Bool;	

	// Fill type to string map
	for (auto it = keywordToTokenType.begin(); it != keywordToTokenType.end(); it++)
		tokenTypeToString[it->second] = it->first;
	tokenTypeToString[TokenType::Unknown] = "?";

	keywordToTokenType["true"] = TokenType::Bool;
	keywordToTokenType["false"] = TokenType::Bool;


	auto Double = TokenType::Double;
	auto Bool   = TokenType::Bool;
	auto Str    = TokenType::Str;

	////////////////////////////////
	//
	//  FUNCTIONS :
	//
	///////////////////////////////

	// returnNumber(number)

	FCT_BEGIN(Double, "returnNumber", Double)
		RETURN( ARG(0) )
	FCT_END

	// sin(number)
	FCT_BEGIN(Double, "sin", Double)
		RETURN( sin(ARG(0)) )
	FCT_END

	// cos(number)
	FCT_BEGIN(Double, "cos", Double)
		RETURN( cos(ARG(0)) )
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
		RETURN( sqrt(ARG(0)) )
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
	
	// bool(number)
	FCT_BEGIN(Bool, "bool", Double)
		RETURN( (bool)ARG(0))
	FCT_END

	// mod(number, number)
	FCT_BEGIN(Double, "mod", Double, Double)
		RETURN( (int)ARG(0) % (int)ARG(1) )
	FCT_END

	// pow(number)
	FCT_BEGIN(Double, "pow", Double, Double)
		RETURN( pow(ARG(0), ARG(1)) )
	FCT_END
	
	// secondDegreePolynomial(a: number, x: number, b:number, y:number, c:number)
	FCT_BEGIN(Double, "secondDegreePolynomial", Double, Double, Double, Double, Double)
		const auto value = 
			(double)ARG(0) * pow((double)ARG(1), 2) * +  // ax² +
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
	

	// time()
	FCT_BEGIN(Double, "time")
		time_t rawtime;
		struct tm* timeinfo;
		time(&rawtime);
		localtime_s(timeinfo, &rawtime);
		RETURN( (double)rawtime )
	FCT_END
	

	////////////////////////////////
	//
	//  OPERATORS :
	//
	///////////////////////////////

	// operator+(number, number)
	OPERATOR_BEGIN(Double, "+", Double, 10u, Double, ICON_FA_PLUS " Add")
		if (_args[0]->getType() == Type_Number)
			RETURN( (double)ARG(0) + (double)ARG(1))
		else
			FAIL
	OPERATOR_END
	
	// operator-(number, number)	
	OPERATOR_BEGIN(Double, "-", Double, 10u, Double, ICON_FA_MINUS " Subtract")
		RETURN( (double)ARG(0) - (double)ARG(1) )
	OPERATOR_END
	
	// operator/(number, number)
	OPERATOR_BEGIN(Double, "/", Double, 20u, Double, ICON_FA_DIVIDE " Divide");
		RETURN( (double)ARG(0) / (double)ARG(1) )
	OPERATOR_END
	

	// operator*(number, number)
	OPERATOR_BEGIN(Double, "*", Double, 20u, Double, ICON_FA_TIMES " Multiply")
		RETURN( (double)ARG(0) * (double)ARG(1) )
	OPERATOR_END

	// operator!(boolean)
	OPERATOR_BEGIN(Bool, "!", Bool, 5u, Bool, "! not");
		RETURN( !(bool)ARG(0) )
	OPERATOR_END

	// operator=(number)
	OPERATOR_BEGIN(Double, "=", Double, 1u, Double, ICON_FA_EQUALS " Assign");
		RETURN( (double)ARG(0))
	OPERATOR_END
}
