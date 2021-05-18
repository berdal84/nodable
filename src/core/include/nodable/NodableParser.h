#pragma once

#include <nodable/Nodable.h> // forward declarations
#include <nodable/Language.h>
#include <nodable/Parser.h>
#include <string>

namespace Nodable {

    // forward declarations
    class Language;

	class NodableParser : public Parser
	{
	public:
		explicit NodableParser(const Language* _language): Parser(_language){}
		~NodableParser() = default;

    protected:
        virtual bool parseToken(std::string::const_iterator& start, const std::string::const_iterator& end);
    };

}