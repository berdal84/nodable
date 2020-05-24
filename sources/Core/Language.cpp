#include "Language.h"
#include "Member.h"
#include <type_traits>
#include <time.h>
#include <vector>

using namespace Nodable;

FunctionArg::FunctionArg(TokenType_ _type, std::string _name) {
	type = _type;
	name = _name;
}

FunctionPrototype::FunctionPrototype(std::string _identifier, TokenType_ _type):
	identifier(_identifier),
	type(_type),
	call(NULL)
{

}

void FunctionPrototype::pushArg(TokenType_ _type, std::string _name) {
	if (_name == "")
		_name = "arg_" + std::to_string(args.size());
	args.push_back( FunctionArg(_type, _name) );
}

bool FunctionPrototype::match(FunctionPrototype& _other) {	

	if (identifier != _other.identifier)
		return false;

	if (args.size() != _other.args.size())
		return false;

	for (size_t i = 0; i < args.size(); i++) {
		if (args[i].type != _other.args[i].type)
			return false;
	}

	return true;
}

const std::string& FunctionPrototype::getIdentifier()const
{
	return this->identifier;
}

const std::string Nodable::FunctionPrototype::getSignature() const
{
	std::string result = identifier + "(";

	for (auto it = args.begin(); it != args.end(); it++) {

		if (it != args.begin())
			result.append(", ");

		if ((*it).type == TokenType_Number)
			result.append("num");
		else if ((*it).type == TokenType_String)
			result.append("str");
		else if ((*it).type == TokenType_Boolean)
			result.append("bool");
		else
			result.append("?");
				
	}
	
	result.append(")");

	return result;
}

const std::vector<FunctionArg> FunctionPrototype::getArgs() const
{
	return this->args;
}

const TokenType_ FunctionPrototype::getType() const
{
	return type;
}

const Language* Language::NODABLE = nullptr;

const Language* Language::Nodable() {

	if (Language::NODABLE != nullptr)
		return Language::NODABLE;

	auto language = new Language("Nodable");

	language->letters   = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_";

	language->numbers   = "0123456789.";

	language->addOperator( Operator("=", 1u) );
	language->addOperator( Operator("!", 5u) );
	language->addOperator( Operator("-", 10u) );
	language->addOperator( Operator("+", 10u) );
	language->addOperator( Operator("/", 20u) );
	language->addOperator( Operator("*", 20u) );

	language->keywords["true"]  = TokenType_Boolean;
	language->keywords["false"] = TokenType_Boolean;

	/* Define a serialize functions for this language */

	language->serializeFunction = [](
		FunctionPrototype _prototype,
		std::vector<const Member*> _args)
		->std::string
	{
		std::string expr;
		expr.append(_prototype.getIdentifier());
		expr.append("( ");

		for (auto it = _args.begin(); it != _args.end(); it++) {
			expr.append((*it)->getSourceExpression());

			if (*it != _args.back()) {
				expr.append(", ");
			}
		}

		expr.append(" )");
		return expr;
	};
	

	/* Function library */

	{
		FunctionPrototype proto("returnNumber", TokenType_Number);
		proto.pushArg(TokenType_Number);

		proto.call = [](Member* _result, const std::vector<const Member*>& _args)->int {
			_result->set(_args[0]);
			return 0;
		};

		language->addToAPI(proto);
	}

	{
		FunctionPrototype proto("sin", TokenType_Number);
		proto.pushArg(TokenType_Number);

		proto.call = [](Member* _result, const std::vector<const Member*>& _args)->int {
			_result->set( sin(*_args[0]) );
			return 0;
		};

		language->addToAPI(proto);
	}

	{
		FunctionPrototype proto("cos", TokenType_Number);
		proto.pushArg(TokenType_Number);

		proto.call = [](Member* _result, const std::vector<const Member*>& _args)->int {
			_result->set( cos(*_args[0]) );
			return 0;
		};

		language->addToAPI(proto);
	}

	{
		FunctionPrototype proto("add", TokenType_Number);
		proto.pushArg(TokenType_Number);
		proto.pushArg(TokenType_Number);

		proto.call = [](Member* _result, const std::vector<const Member*>& _args)->int {
			_result->set( (double)*_args[0] + (double)*_args[1]);
			return 0;
		};

		language->addToAPI(proto);
	}

	{
		FunctionPrototype proto("minus", TokenType_Number);
		proto.pushArg(TokenType_Number);
		proto.pushArg(TokenType_Number);

		proto.call = [](Member* _result, const std::vector<const Member*>& _args)->int {
			_result->set((double)*_args[0] - (double)*_args[1]);
			return 0;
		};

		language->addToAPI(proto);
	}

	{
		FunctionPrototype proto("mult", TokenType_Number);
		proto.pushArg(TokenType_Number);
		proto.pushArg(TokenType_Number);

		proto.call = [](Member* _result, const std::vector<const Member*>& _args)->int {
			_result->set((double)*_args[0] * (double)*_args[1]);
			return 0;
		};

		language->addToAPI(proto);
	}

	{
		FunctionPrototype proto("sqrt", TokenType_Number);
		proto.pushArg(TokenType_Number);
		proto.call = [](Member* _result, const std::vector<const Member*>& _args)->int {
			_result->set( sqrt(*_args[0]) );
			return 0;
		};
		language->addToAPI(proto);
	}

	{
		FunctionPrototype proto("not", TokenType_Boolean);
		proto.pushArg(TokenType_Boolean);
		proto.call = [](Member* _result, const std::vector<const Member*>& _args)->int {
			_result->set( !*_args[0] );
			return 0;
		};
		language->addToAPI(proto);
	}

	{
		FunctionPrototype proto("or", TokenType_Boolean);
		proto.pushArg(TokenType_Boolean);
		proto.pushArg(TokenType_Boolean);

		proto.call = [](Member* _result, const std::vector<const Member*>& _args)->int {
			_result->set( (bool*)_args[0] || *_args[1] );
			return 0;
		};

		language->addToAPI(proto);
	}

	{
		FunctionPrototype proto("and", TokenType_Boolean);
		proto.pushArg(TokenType_Boolean);
		proto.pushArg(TokenType_Boolean);

		proto.call = [](Member* _result, const std::vector<const Member*>& _args)->int {
			_result->set( (bool)*_args[0] && *_args[1] );
			return 0;
		};

		language->addToAPI(proto);
	}

	{
		FunctionPrototype proto("xor", TokenType_Boolean);
		proto.pushArg(TokenType_Boolean);
		proto.pushArg(TokenType_Boolean);

		proto.call = [](Member* _result, const std::vector<const Member*>& _args)->int {
			_result->set(
				( (bool)*_args[0] && !(bool)*_args[1]) ||
				(!(bool)*_args[0] &&  (bool)*_args[1]));
			return 0;
		};

		language->addToAPI(proto);
	}

	{
		FunctionPrototype proto("bool", TokenType_Boolean);
		proto.pushArg(TokenType_Number);
		proto.call = [](Member* _result, const std::vector<const Member*>& _args)->int {
			_result->set((bool)*_args[0]);
			return 0;
		};
		language->addToAPI(proto);
	}

	{
		FunctionPrototype proto("mod", TokenType_Number);
		proto.pushArg(TokenType_Number);
		proto.pushArg(TokenType_Number);
		proto.call = [](Member* _result, const std::vector<const Member*>& _args)->int {
			_result->set( (int)*_args[0] % (int)*_args[1] );
			return 0;
		};
		language->addToAPI(proto);
	}

	{
		FunctionPrototype proto("pow", TokenType_Number);
		proto.pushArg(TokenType_Number);
		proto.pushArg(TokenType_Number);
		proto.call = [](Member* _result, const std::vector<const Member*>& _args)->int {
			const auto value = pow( *_args[0], *_args[1]);
			_result->set(value);
			return 0;
		};

		language->addToAPI(proto);
	}

	{
		FunctionPrototype proto("secondDegreePolynomial", TokenType_Number);
		proto.pushArg(TokenType_Number, "a");
		proto.pushArg(TokenType_Number, "x");
		proto.pushArg(TokenType_Number, "b");
		proto.pushArg(TokenType_Number, "y");
		proto.pushArg(TokenType_Number, "c");
		proto.call = [](Member* _result, const std::vector<const Member*>& _args)->int {
			const auto value = (double)*_args[0] * pow((double)*_args[1], 2) *  + // ax² +
							   (double)*_args[2] * (double)*_args[3] +            // by +
				               (double)*_args[4];                                 // c
			_result->set(value); 
			return 0;
		};

		language->addToAPI(proto);
	}

	{
		FunctionPrototype proto("DNAtoProtein", TokenType_String);
		proto.pushArg(TokenType_String);
		proto.call = [](Member* _result, const std::vector<const Member*>& _args)->int {
			
			auto baseChain = (std::string)*_args[0];
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

			for (size_t i = 0; i < baseChain.size() / 3; i++ ) {
				auto found = table.find( baseChain.substr(i, 3));
				if (found != table.end() )
					protein += found->second;
			}

			_result->set(protein);
			return 0;
		};
		language->addToAPI(proto);
	}

	{
		FunctionPrototype proto("time", TokenType_Number);
		proto.call = [](Member* _result, const std::vector<const Member*>& _args)->int {
			time_t rawtime;
			struct tm* timeinfo;
			time(&rawtime);
			timeinfo = localtime(&rawtime);
			_result->set((double)rawtime);
			return 0;
		};

		language->addToAPI(proto);
	}

	Language::NODABLE = language;

	return language;
}

void Language::addOperator( Operator _operator) {

	auto item = std::pair<std::string, Operator>(_operator.identifier, _operator);
	operators.insert(item);
}

unsigned short Language::getOperatorPrecedence(const std::string& _identifier)const {

	return operators.at(_identifier).precedence;
}

bool  Language::needsToBeEvaluatedFirst(std::string op, std::string nextOp)const {
	return getOperatorPrecedence(op) > getOperatorPrecedence(nextOp);
}

std::string Nodable::Language::getOperatorsAsString() const
{
	std::string result;

	for (auto it = operators.begin(); it != operators.end(); it++) {
		result.append((*it).second.identifier);
	}

	return result;
}

const FunctionPrototype* Nodable::Language::find(FunctionPrototype& _prototype) const
{
	auto predicate = [&](FunctionPrototype p) {
		return p.match(_prototype);
	};

	auto it = std::find_if(api.begin(), api.end(), predicate);

	if (it != api.end())
		return &*it;

	return nullptr;
}

void Nodable::Language::addToAPI(FunctionPrototype prototype)
{
	this->api.push_back(prototype);
}
