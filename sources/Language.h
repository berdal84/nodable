#pragma once

#include "Nodable.h"   // for constants and forward declarations
#include <map>

namespace Nodable {

	struct Operator {

		Operator(std::string _identifier,
			     unsigned short _precedence) :
			identifier(_identifier),
			precedence(_precedence) {}

		std::string identifier;
		unsigned short precedence;
	};

	class Language {
	public:
		Language() :numbers(), letters(), brackets() {};
		~Language() {};		

		void addOperator(Operator);
		unsigned short getOperatorPrecedence(std::string& _identifier)const;
		std::string getOperatorsAsString()const;

		/* Some language reference constants*/
		static const Language* NODABLE;

	private:

		/* To generate the Nodable Language reference */
		static const Language* Nodable();

		/* New language generators will be found here later... */
		/* ex: static const Language& CPlusPlus(); */

	public:
		std::string numbers;
		std::string letters;

	private:
		std::vector<char> brackets;
		std::map<std::string, Operator> operators;
	};

}
