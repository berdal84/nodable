#include "Language.h"
#include "Member.h"
#include <type_traits>
using namespace Nodable;


template<class F>
struct function_traits;

// function pointer
template<class R, class... Args>
struct function_traits<R(*)(Args...)> : public function_traits<R(Args...)>
{};

template<class R, class... Args>
struct function_traits<R(Args...)>
{
	using return_type = R;

	static constexpr std::size_t arity = sizeof...(Args);

	template <std::size_t N>
	struct argument
	{
		static_assert(N < arity, "error: invalid parameter index.");
		using type = typename std::tuple_element<N, std::tuple<Args...>>::type;
	};
};


float free_function(const std::string & a, int b)
{
	return (float)a.size() / b;
}


FunctionArg::FunctionArg(TokenType_ _type, std::string _name) {
	type = _type;
	name = _name;
}

FunctionPrototype::FunctionPrototype(std::string _identifier, TokenType_ _type):
	identifier(_identifier),
	type(_type),
	nativeFunction(NULL)
{

}

void FunctionPrototype::pushArg(TokenType_ _type) {

	std::string argName = "arg_" + std::to_string(args.size());
	args.push_back( FunctionArg(_type, argName) );
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

	auto language = new Language();

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

	using Traits = function_traits<decltype(cos)>;

	Traits::argument<0>::type var = 5;


	/* Function library */

	{
		FunctionPrototype proto("pass", TokenType_Number);
		proto.pushArg(TokenType_Number);

		proto.nativeFunction = [](Member* result, const Member* arg0, const Member* arg1)->void {
			result->set(arg0->as<double>());
		};

		language->pushFunc(proto);
	}

	{
		FunctionPrototype proto("sin", TokenType_Number);
		proto.pushArg(TokenType_Number);

		proto.nativeFunction = [](Member* result, const Member* arg0, const Member* arg1)->void {
			result->set( sin(arg0->as<double>()) );
		};

		language->pushFunc(proto);
	}

	{
		FunctionPrototype proto("cos", TokenType_Number);
		proto.pushArg(TokenType_Number);

		proto.nativeFunction = [](Member* result, const Member* arg0, const Member* arg1)->void {
			result->set(cos(arg0->as<double>()));
		};

		language->pushFunc(proto);
	}

	{
		FunctionPrototype proto("add", TokenType_Number);
		proto.pushArg(TokenType_Number);
		proto.pushArg(TokenType_Number);

		proto.nativeFunction = [](Member* result, const Member* arg0, const Member* arg1)->void {
			result->set( arg0->as<double>() + arg1->as<double>());
		};

		language->pushFunc(proto);
	}

	{
		FunctionPrototype proto("minus", TokenType_Number);
		proto.pushArg(TokenType_Number);
		proto.pushArg(TokenType_Number);

		proto.nativeFunction = [](Member* result, const Member* arg0, const Member* arg1)->void {
			result->set(arg0->as<double>() - arg1->as<double>());
		};

		language->pushFunc(proto);
	}

	{
		FunctionPrototype proto("mult", TokenType_Number);
		proto.pushArg(TokenType_Number);
		proto.pushArg(TokenType_Number);

		proto.nativeFunction = [](Member* result, const Member* arg0, const Member* arg1)->void {
			result->set(arg0->as<double>() * arg1->as<double>());
		};

		language->pushFunc(proto);
	}

	{
		FunctionPrototype proto("sqrt", TokenType_Number);
		proto.pushArg(TokenType_Number);
		proto.nativeFunction = [](Member* result, const Member* arg0, const Member* arg1)->void {
			result->set( sqrt(arg0->as<double>()));
		};
		language->pushFunc(proto);
	}

	{
		FunctionPrototype proto("pow", TokenType_Number);
		proto.pushArg(TokenType_Number);
		proto.pushArg(TokenType_Number);
		proto.nativeFunction = [](Member* result, const Member* arg0, const Member* arg1)->void {
			const auto value = pow(arg0->as<double>(), arg1->as<double>());
			result->set(value);
		};

		language->pushFunc(proto);
	}

	{
		FunctionPrototype proto("DNAtoAninoAcid", TokenType_String);
		proto.pushArg(TokenType_String);
		proto.nativeFunction = [](Member* result, const Member* arg0, const Member* arg1)->void {
			
			std::string value = "<TODO>";

			if (arg0->as<std::string>() == "UAA" ||
				arg0->as<std::string>() == "UAG" ||
				arg0->as<std::string>() == "UGA") {

				value = "Stop";
			}

			result->set(value);
		};
		language->pushFunc(proto);
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
	return getOperatorPrecedence(op) >= getOperatorPrecedence(nextOp);
}

std::string Nodable::Language::getOperatorsAsString() const
{
	std::string result;

	for (auto it = operators.begin(); it != operators.end(); it++) {
		result.append((*it).second.identifier);
	}

	return result;
}

const FunctionPrototype* Nodable::Language::findFunctionPrototype(FunctionPrototype& _prototype) const
{
	auto predicate = [&](FunctionPrototype p) {
		return p.match(_prototype);
	};

	auto it = std::find_if(functionPrototypes.begin(), functionPrototypes.end(), predicate);

	if (it != functionPrototypes.end())
		return &*it;

	return nullptr;
}

void Nodable::Language::pushFunc(FunctionPrototype prototype)
{
	this->functionPrototypes.push_back(prototype);
}
