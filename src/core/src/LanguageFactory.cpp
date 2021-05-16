#include "LanguageFactory.h"

#include <NodableParser.h>
#include <NodableSerializer.h>
#include "NodableLanguage.h"

using namespace Nodable::core;

const Language* LanguageFactory::Nodable = nullptr;

const Language* LanguageFactory::GetNodable()
{
    if (LanguageFactory::Nodable == nullptr)
    {
        LanguageFactory::Nodable = new NodableLanguage();
    }
    return LanguageFactory::Nodable;
}