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

LanguageNodable::LanguageNodable(): Language("Nodable")
{

	letters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_";
    numbers = "0123456789.";

	keywords["true"]  = TokenType_Boolean;
	keywords["false"] = TokenType_Boolean;


	////////////////////////////////
	//
	//  FUNCTIONS :
	//
	///////////////////////////////

	// returnNumber(number)
	FCT_BEGIN( TokenType_Number, "returnNumber", TokenType_Number)
		RETURN( _args[0] )
	FCT_END

	// sin(number)
	FCT_BEGIN(TokenType_Number, "sin", TokenType_Number)
		RETURN( sin(*_args[0]) )
	FCT_END

	// cos(number)
	FCT_BEGIN(TokenType_Number, "cos", TokenType_Number)
		RETURN( cos(*_args[0]) )
	FCT_END

	// add(number)
	FCT_BEGIN(TokenType_Number, "add", TokenType_Number, TokenType_Number)	
		RETURN((double)*_args[0] + (double)*_args[1])
	FCT_END

	// minus(number)
	FCT_BEGIN(TokenType_Number, "minus", TokenType_Number, TokenType_Number)
		RETURN( (double)*_args[0] - (double)*_args[1] )
	FCT_END

	// mult(number)
	FCT_BEGIN(TokenType_Number, "mult", TokenType_Number, TokenType_Number)
		RETURN( (double)*_args[0] * (double)*_args[1] )
	FCT_END

	// sqrt(number)
	FCT_BEGIN(TokenType_Number, "sqrt", TokenType_Number)
		RETURN( sqrt(*_args[0]) )
	FCT_END

	// not(boolean)
	FCT_BEGIN(TokenType_Boolean, "not", TokenType_Boolean)
		RETURN( !*_args[0] )
	FCT_END

	// or(boolean, boolean)
	FCT_BEGIN(TokenType_Boolean, "or", TokenType_Boolean, TokenType_Boolean)
		RETURN( (bool*)_args[0] || *_args[1] )
	FCT_END
	
	// and(boolean, boolean)
	FCT_BEGIN(TokenType_Boolean, "and", TokenType_Boolean, TokenType_Boolean)
		RETURN( (bool)*_args[0] && *_args[1] )
	FCT_END

	// xor(boolean, boolean)
	FCT_BEGIN(TokenType_Boolean, "xor", TokenType_Boolean, TokenType_Boolean)
		RETURN(
		((bool)*_args[0] && !(bool)*_args[1]) ||
		(!(bool)*_args[0] && (bool)*_args[1]) )
	FCT_END
	
	// bool(number)
	FCT_BEGIN(TokenType_Boolean, "bool", TokenType_Number)
		RETURN( (bool)*_args[0] )
	FCT_END

	// mod(number, number)
	FCT_BEGIN(TokenType_Number, "mod", TokenType_Number, TokenType_Number)
		RETURN( (int)*_args[0] % (int)*_args[1] )
	FCT_END

	// pow(number)
	FCT_BEGIN(TokenType_Number, "pow", TokenType_Number, TokenType_Number)
		RETURN( pow(*_args[0], *_args[1]) )
	FCT_END
	
	// secondDegreePolynomial(a: number, x: number, b:number, y:number, c:number)
	FCT_BEGIN(TokenType_Number, "secondDegreePolynomial", TokenType_Number, TokenType_Number, TokenType_Number, TokenType_Number, TokenType_Number)
		const auto value = 
			(double)*_args[0] * pow((double)*_args[1], 2) * +  // ax² +
			(double)*_args[2] * (double)*_args[3] +            // by +
			(double)*_args[4];                                 // c
	RETURN(value)
	FCT_END

	// DNAtoProtein(string)
	FCT_BEGIN(TokenType_String, "DNAtoProtein", TokenType_String)
		auto baseChain = (std::string) * _args[0];
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
	FCT_BEGIN(TokenType_Number, "time")
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
	OPERATOR_BEGIN(TokenType_Number, "+", TokenType_Number, 10u, TokenType_Number, ICON_FA_PLUS " Add")
		if (_args[0]->getType() == Type_Number)
			RETURN( (double)*_args[0] + (double)*_args[1])
		else
			FAIL
	OPERATOR_END
	
	// operator-(number, number)	
	OPERATOR_BEGIN(TokenType_Number, "-", TokenType_Number, 10u, TokenType_Number, ICON_FA_MINUS " Subtract")
		RETURN( (double)*_args[0] - (double)*_args[1] )
	OPERATOR_END
	
	// operator/(number, number)
	OPERATOR_BEGIN(TokenType_Number, "/", TokenType_Number, 20u, TokenType_Number, ICON_FA_DIVIDE " Divide");
		RETURN( (double)*_args[0] / (double)*_args[1] )
	OPERATOR_END
	

	// operator*(number, number)
	OPERATOR_BEGIN(TokenType_Number, "*", TokenType_Number, 20u, TokenType_Number, ICON_FA_TIMES " Multiply")
		RETURN( (double)*_args[0] * (double)*_args[1] )
	OPERATOR_END

	// operator!(boolean)
	OPERATOR_BEGIN(TokenType_Boolean, "!", TokenType_Boolean, 5u, TokenType_Boolean, "! not");
		RETURN( !(bool)*_args[0] )
	OPERATOR_END

	// operator=(number)
	OPERATOR_BEGIN(TokenType_Number, "=", TokenType_Number, 1u, TokenType_Number, ICON_FA_EQUALS " Assign");
		RETURN( (double)*_args[0])
	OPERATOR_END
}