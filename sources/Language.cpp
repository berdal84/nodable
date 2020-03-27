#include "Language.h"

using namespace Nodable;

const Language* Language::NODABLE = Language::Nodable();

const Language* Language::Nodable() {

	auto language = new Language();

	language->letters   = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_";

	language->numbers   = "0123456789.";

	language->addOperator( Operator( "=", 0u) );
	language->addOperator( Operator("!", 5u));
	language->addOperator( Operator("-", 10u));
	language->addOperator( Operator("+", 20u));
	language->addOperator( Operator("/", 30u));
	language->addOperator( Operator("*", 40u));

	return language;
}

void Language::addOperator( Operator _operator) {

	auto item = std::pair<std::string, Operator>(_operator.identifier, _operator);
	operators.insert(item);
}

unsigned short Language::getOperatorPrecedence(std::string& _identifier)const {

	return operators.at(_identifier).precedence;
}

std::string Nodable::Language::getOperatorsAsString() const
{
	std::string result;

	for (auto it = operators.begin(); it != operators.end(); it++) {
		result.append((*it).second.identifier);
	}

	return result;
}
