#include <nodable/LanguageFactory.h>

#include <nodable/NodableParser.h>
#include <nodable/NodableSerializer.h>
#include <nodable/NodableLanguage.h>

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