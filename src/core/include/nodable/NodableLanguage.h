#pragma once
#include <nodable/Language.h>

namespace Nodable::core {

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

