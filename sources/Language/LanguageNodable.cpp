#include "LanguageNodable.h"
#include "Member.h"
#include <time.h>
#include "IconsFontAwesome5.h"
#include <cmath>
#include <iostream>

using namespace Nodable;

std::string LanguageNodable::serialize(
	const FunctionSignature&   _signature,
	std::vector<Member*> _args) const
{
	std::string expr;
	expr.append(_signature.getIdentifier());
	expr.append(serialize(TokenType::LBracket) + " ");

	for (auto it = _args.begin(); it != _args.end(); it++) {
		expr.append((*it)->getSourceExpression());

		if (*it != _args.back()) {
			expr.append(serialize(TokenType::Separator));
			expr.append(" ");
		}
	}

	expr.append(" " + serialize(TokenType::RBracket));
	return expr;

}

std::string LanguageNodable::serialize(const FunctionSignature& _signature) const {

	std::string result = _signature.getIdentifier() + serialize(TokenType::LBracket);
	auto args = _signature.getArgs();

	for (auto it = args.begin(); it != args.end(); it++) {

		if (it != args.begin()) {
			result.append(serialize(TokenType::Separator));
			result.append(" ");

		}
		const auto argType = (*it).type;
		result.append( serialize(argType) );

	}

	result.append( serialize(TokenType::RBracket) );

	return result;

}

std::string LanguageNodable::serialize(const TokenType& _type) const {
	return dictionnary.convert(_type);
}

std::string LanguageNodable::serializeBinaryOp(const Operator* _op, std::vector<Member*> _args, const Operator* _leftOp, const Operator* _rightOp) const
{
	std::string result;

	// Left part of the expression
	{
		bool needBrackets = _leftOp && /*( _leftOp->getType() == Operator::Type::Unary ||*/ !hasHigherPrecedenceThan(_leftOp, _op) ;
		if (needBrackets) result.append( serialize(TokenType::LBracket) + " ");
		result.append(_args[0]->getSourceExpression());
		if (needBrackets) result.append(" " + serialize(TokenType::RBracket));
	}

	// Operator
	result.append(" ");
	result.append(_op->identifier);
	result.append(" ");

	// Right part of the expression
	{
		bool needBrackets = _rightOp && (  _rightOp->getType() == Operator::Type::Unary || !hasHigherPrecedenceThan(_rightOp, _op) );

		if (needBrackets)
        {
		    result.append(serialize(TokenType::LBracket) + " ");
        }

        // TODO: do a recursive call to generate expression instead of getting it from Member class.
		result.append(_args[1]->getSourceExpression());

		if (needBrackets)
        {
		    result.append(" " + serialize(TokenType::RBracket));
        }
	}

	return result;
}

std::string LanguageNodable::serializeUnaryOp(const Operator* _op, std::vector<Member*> _args, const Operator* _innerOp) const
{
	std::string result;

	// operator ( ... innerOperator ... )   ex:   -(a+b)

	// Operator
	result.append(_op->identifier);

	// Inner part of the expression
	{
		bool needBrackets = _innerOp;

		if (needBrackets)
		{
            result.append(serialize(TokenType::LBracket) + " ");
		}

		// TODO: do a recursive call to generate expression instead of getting it from Member class.
		result.append(_args[0]->getSourceExpression());

		if (needBrackets)
        {
		    result.append(" " + serialize(TokenType::RBracket));
        }
	}

	return result;
}

const FunctionSignature LanguageNodable::createBinOperatorSignature(
	Type _type,
	std::string _identifier,
	Type _ltype,
	Type _rtype) const
{
	auto tokType  = typeToTokenType(_type);
	auto tokLType = typeToTokenType(_ltype);
	auto tokRType = typeToTokenType(_rtype);

	FunctionSignature signature("operator" + _identifier, tokType);

	signature.pushArgs(tokLType, tokRType);

	return signature;
}

const FunctionSignature LanguageNodable::createUnaryOperatorSignature(
	Type _type,
	std::string _identifier,
	Type _ltype) const
{
	auto tokType = typeToTokenType(_type);
	auto tokLType = typeToTokenType(_ltype);

	FunctionSignature signature("operator" + _identifier, tokType);

	signature.pushArgs(tokLType);

	return signature;
}

LanguageNodable::LanguageNodable(): Language("Nodable")
{

	// Setup dictionnary:
	dictionnary.insert(std::regex("^(true|false)")       , TokenType::Boolean );
	dictionnary.insert(std::regex("^(\"[a-zA-Z0-9 ]+\")"), TokenType::String );
	dictionnary.insert(std::regex("^[a-zA-Z_]+")         , TokenType::Symbol);
	dictionnary.insert(std::regex("^(0|([1-9][0-9]*))(\\.[0-9]+)?") , TokenType::Double);

	dictionnary.insert("bool"  , TokenType::BooleanType);
	dictionnary.insert("string", TokenType::StringType);
	dictionnary.insert("number", TokenType::DoubleType);
	dictionnary.insert("any"   , TokenType::AnyType);

	dictionnary.insert( 
		std::regex("^(&&|[|][|]|==|>|<(?!(=))|<=>|=>|=(?!(>|=))|<=(?!(>))|>=|[+]|[-]|[/]|[*]|[!])"),
		TokenType::Operator);

	dictionnary.insert(
		std::regex("^((//(.+?)$)|(/\\*(.+?)\\*/)|[ \t])"), // comments (single line, multi) and space and tab
		TokenType::Ignore); 

	dictionnary.insert("("	     , TokenType::LBracket);
	dictionnary.insert(")"	     , TokenType::RBracket);
	dictionnary.insert(","	     , TokenType::Separator);
	dictionnary.insert(";"      , TokenType::EndOfInstruction);

	// To easily declare types
	auto Double = TokenType::DoubleType;
	auto Bool   = TokenType::BooleanType;
	auto Str    = TokenType::StringType;

	////////////////////////////////
	//
	//  FUNCTIONS :
	//
	///////////////////////////////

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
		RETURN( (int)ARG(0) % (int)ARG(1) )
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
	

	////////////////////////////////
	//
	//  OPERATORS :
	//
	///////////////////////////////

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
		_args[0]->set(ARG(1));
		RETURN((double)ARG(1))
	OPERATOR_END

    // string operator=(string, string)
    BINARY_OP_BEGIN(Str, "=", Str, Str, 0u, ICON_FA_EQUALS " Assign")
            _args[0]->set(ARG(1));
            RETURN((std::string)ARG(1))
    OPERATOR_END

	// bool operator=(bool, bool)
	BINARY_OP_BEGIN(Bool, "=", Bool, Bool, 0u, ICON_FA_EQUALS " Assign")
			_args[0]->set(ARG(1));
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

const TokenType LanguageNodable::typeToTokenType(Type _type)const
{
	// TODO: build a map in constructor
	switch (_type)
	{
	case Type::Boolean: return TokenType::BooleanType;
	case Type::Double:  return TokenType::DoubleType;
	case Type::String:  return TokenType::StringType;
	default:
		return TokenType::AnyType;
		break;
	}
}

const Type LanguageNodable::tokenTypeToType(TokenType _tokenType)const
{
	// TODO: build a map in constructor
	switch (_tokenType)
	{
	case TokenType::BooleanType: return Type::Boolean;
	case TokenType::DoubleType:  return Type::Double;
	case TokenType::StringType:  return Type::String;
	default:
		return Type::Any;
		break;
	}
}

std::string LanguageNodable::serialize(const Member *) const
{

    return std::string();
}
