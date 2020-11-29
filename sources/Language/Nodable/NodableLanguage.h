#pragma once
#include "Language/Common/Language.h"

namespace Nodable {

	/*
		The role of this class is to define a first language for Nodable.
	*/
	class NodableLanguage : public Language
	{
	public:
		NodableLanguage();
        ~NodableLanguage() override = default;
	};
}

