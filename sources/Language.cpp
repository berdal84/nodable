#include "Language.h"

using namespace Nodable;

const Language& Language::Nodable() {

	static Language language;

	language.letters   = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_";
	language.operators = "!+-*/=";
	language.numbers   = "0123456789.";

	return language;
}


unsigned short Language::GetOperatorPrecedence(std::string _operator) {

	if (_operator == "=") return 0u;
	if (_operator == "!") return 5u;
	if (_operator == "-") return 10u;
	if (_operator == "+") return 20u;
	if (_operator == "/") return 30u;
	if (_operator == "*") return 40u;
}
