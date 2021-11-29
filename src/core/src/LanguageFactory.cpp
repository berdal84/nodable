#include <nodable/LanguageFactory.h>
#include <nodable/NodableLanguage.h>

using namespace Nodable;

const Language* LanguageFactory::Nodable = nullptr;

const Language* LanguageFactory::GetNodable()
{
    if (LanguageFactory::Nodable == nullptr)
    {
        LanguageFactory::Nodable = new NodableLanguage();
    }
    return LanguageFactory::Nodable;
}