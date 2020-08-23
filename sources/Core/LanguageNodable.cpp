#include "LanguageNodable.h"
#include "Member.h"
#include <time.h>

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


	/* Function library */

	{
		FunctionSignature signature("returnNumber", TokenType_Number);
		signature.pushArg(TokenType_Number);

		auto implementation = [](Member* _result, const std::vector<const Member*>& _args)->int {
			_result->set(_args[0]);
			return 0;
		};

		Function f(signature, implementation);

		addToAPI(f);
	}

	{
		FunctionSignature signature("sin", TokenType_Number);
		signature.pushArg(TokenType_Number);

		auto implementation = [](Member* _result, const std::vector<const Member*>& _args)->int {
			_result->set(sin(*_args[0]));
			return 0;
		};

		Function f(signature, implementation);

		addToAPI(f);
	}

	{
		FunctionSignature signature("cos", TokenType_Number);
		signature.pushArg(TokenType_Number);

		auto implementation = [](Member* _result, const std::vector<const Member*>& _args)->int {
			_result->set(cos(*_args[0]));
			return 0;
		};

		Function f(signature, implementation);

		addToAPI(f);
	}

	{
		FunctionSignature signature("add", TokenType_Number);
		signature.pushArg(TokenType_Number);
		signature.pushArg(TokenType_Number);

		auto implementation = [](Member* _result, const std::vector<const Member*>& _args)->int {
			_result->set((double)*_args[0] + (double)*_args[1]);
			return 0;
		};

		Function f(signature, implementation);

		addToAPI(f);
	}

	{
		FunctionSignature signature("minus", TokenType_Number);
		signature.pushArg(TokenType_Number);
		signature.pushArg(TokenType_Number);

		auto implementation = [](Member* _result, const std::vector<const Member*>& _args)->int {
			_result->set((double)*_args[0] - (double)*_args[1]);
			return 0;
		};

		Function f(signature, implementation);

		addToAPI(f);
	}

	{
		FunctionSignature signature("mult", TokenType_Number);
		signature.pushArg(TokenType_Number);
		signature.pushArg(TokenType_Number);

		auto implementation = [](Member* _result, const std::vector<const Member*>& _args)->int {
			_result->set((double)*_args[0] * (double)*_args[1]);
			return 0;
		};

		Function f(signature, implementation);

		addToAPI(f);
	}

	{
		FunctionSignature signature("sqrt", TokenType_Number);
		signature.pushArg(TokenType_Number);
		auto implementation = [](Member* _result, const std::vector<const Member*>& _args)->int {
			_result->set(sqrt(*_args[0]));
			return 0;
		};
		Function f(signature, implementation);

		addToAPI(f);
	}

	{
		FunctionSignature signature("not", TokenType_Boolean);
		signature.pushArg(TokenType_Boolean);
		auto implementation = [](Member* _result, const std::vector<const Member*>& _args)->int {
			_result->set(!*_args[0]);
			return 0;
		};
		Function f(signature, implementation);

		addToAPI(f);
	}

	{
		FunctionSignature signature("or", TokenType_Boolean);
		signature.pushArg(TokenType_Boolean);
		signature.pushArg(TokenType_Boolean);

		auto implementation = [](Member* _result, const std::vector<const Member*>& _args)->int {
			_result->set((bool*)_args[0] || *_args[1]);
			return 0;
		};

		Function f(signature, implementation);

		addToAPI(f);
	}

	{
		FunctionSignature signature("and", TokenType_Boolean);
		signature.pushArg(TokenType_Boolean);
		signature.pushArg(TokenType_Boolean);

		auto implementation = [](Member* _result, const std::vector<const Member*>& _args)->int {
			_result->set((bool)*_args[0] && *_args[1]);
			return 0;
		};

		Function f(signature, implementation);

		addToAPI(f);
	}

	{
		FunctionSignature signature("xor", TokenType_Boolean);
		signature.pushArg(TokenType_Boolean);
		signature.pushArg(TokenType_Boolean);

		auto implementation = [](Member* _result, const std::vector<const Member*>& _args)->int {
			_result->set(
			((bool)*_args[0] && !(bool)*_args[1]) ||
			(!(bool)*_args[0] && (bool)*_args[1]));
			return 0;
		};

		Function f(signature, implementation);

		addToAPI(f);
	}

	{
		FunctionSignature signature("bool", TokenType_Boolean);
		signature.pushArg(TokenType_Number);
		auto implementation = [](Member* _result, const std::vector<const Member*>& _args)->int {
			_result->set((bool)*_args[0]);
			return 0;
		};
		Function f(signature, implementation);

		addToAPI(f);
	}

	{
		FunctionSignature signature("mod", TokenType_Number);
		signature.pushArg(TokenType_Number);
		signature.pushArg(TokenType_Number);
		auto implementation = [](Member* _result, const std::vector<const Member*>& _args)->int {
			_result->set((int)*_args[0] % (int)*_args[1]);
			return 0;
		};
		Function f(signature, implementation);

		addToAPI(f);
	}

	{
		FunctionSignature signature("pow", TokenType_Number);
		signature.pushArg(TokenType_Number);
		signature.pushArg(TokenType_Number);
		auto implementation = [](Member* _result, const std::vector<const Member*>& _args)->int {
			const auto value = pow(*_args[0], *_args[1]);
			_result->set(value);
			return 0;
		};

		Function f(signature, implementation);

		addToAPI(f);
	}

	{
		FunctionSignature signature("secondDegreePolynomial", TokenType_Number);
		signature.pushArg(TokenType_Number, "a");
		signature.pushArg(TokenType_Number, "x");
		signature.pushArg(TokenType_Number, "b");
		signature.pushArg(TokenType_Number, "y");
		signature.pushArg(TokenType_Number, "c");
		auto implementation = [](Member* _result, const std::vector<const Member*>& _args)->int {
			const auto value = (double)*_args[0] * pow((double)*_args[1], 2) * + // ax² +
				(double)*_args[2] * (double)*_args[3] +            // by +
				(double)*_args[4];                                 // c
			_result->set(value);
			return 0;
		};

		Function f(signature, implementation);

		addToAPI(f);
	}

	{
		FunctionSignature signature("DNAtoProtein", TokenType_String);
		signature.pushArg(TokenType_String);
		auto implementation = [](Member* _result, const std::vector<const Member*>& _args)->int {

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

			_result->set(protein);
			return 0;
		};
		Function f(signature, implementation);

		addToAPI(f);
	}

	{
		FunctionSignature signature("time", TokenType_Number);
		auto implementation = [](Member* _result, const std::vector<const Member*>& _args)->int {
			time_t rawtime;
			struct tm* timeinfo;
			time(&rawtime);
			localtime_s(timeinfo, &rawtime);
			_result->set((double)rawtime);
			return 0;
		};

		Function f(signature, implementation);

		addToAPI(f);
	}

	/*
	  Operators :
	*/
	{
		FunctionSignature signature("operator+", TokenType_Number, "+ Add");
		signature.pushArg(TokenType_Number);
		signature.pushArg(TokenType_Number);

		auto implementation = [](Member* _result, const std::vector<const Member*>& _args)->int {

			if (_args[0]->getType() == Type_Number)
				_result->set((double)*_args[0] + (double)*_args[1]);

			return 0;
		};

		Operator op("+", 10u, signature, implementation);

		addOperator(op);
	}

	{
		FunctionSignature signature("operator-", TokenType_Number, "- Subtract");
		signature.pushArg(TokenType_Number);
		signature.pushArg(TokenType_Number);

		auto implementation = [](Member* _result, const std::vector<const Member*>& _args)->int {
			_result->set((double)*_args[0] - (double)*_args[1]);
			return 0;
		};

		Operator op("-", 10u, signature, implementation);

		addOperator(op);
	}

	{
		FunctionSignature signature("operator/", TokenType_Number, "/ Divide");
		signature.pushArg(TokenType_Number);
		signature.pushArg(TokenType_Number);

		auto implementation = [](Member* _result, const std::vector<const Member*>& _args)->int {
			_result->set((double)*_args[0] / (double)*_args[1]);
			return 0;
		};

		Operator op("/", 20u, signature, implementation);

		addOperator(op);
	}

	{
		FunctionSignature signature("operator*", TokenType_Number, "x Multiply");
		signature.pushArg(TokenType_Number);
		signature.pushArg(TokenType_Number);

		auto implementation = [](Member* _result, const std::vector<const Member*>& _args)->int {
			_result->set((double)*_args[0] * (double)*_args[1]);
			return 0;
		};

		Operator op("*", 20u, signature, implementation);

		addOperator(op);
	}

	{
		FunctionSignature signature("operator!", TokenType_Boolean, "! not");
		signature.pushArg(TokenType_Boolean);

		auto implementation = [](Member* _result, const std::vector<const Member*>& _args)->int {
			_result->set(!(bool)*_args[0]);
			return 0;
		};

		Operator op("!", 5u, signature, implementation);

		addOperator(op);
	}

	{
		FunctionSignature signature("operator=", TokenType_Number, "= assign");
		signature.pushArg(TokenType_Number);

		auto implementation = [](Member* _result, const std::vector<const Member*>& _args)->int {
			_result->set((double)*_args[0]);
			return 0;
		};

		Operator op("=", 1u, signature, implementation);

		addOperator(op);
	}

}

