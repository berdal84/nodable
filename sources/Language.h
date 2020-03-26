#pragma once

#include "Nodable.h"   // for constants and forward declarations

namespace Nodable {

	class Language {
	public:
		Language() :numbers(), letters(), operators(), brackets() {};
		~Language() {};

		static unsigned short GetOperatorPrecedence(std::string);

		/* Get the Nodable language definition */
		static const Language& Nodable();

		std::string numbers;
		std::string letters;
		std::string operators;
		std::vector<char> brackets;
	};

}
