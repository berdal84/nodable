#include "LanguageLibrary.h"
#include "Nodable/NodableLanguage.h"

using namespace Nodable;

const Language* LanguageLibrary::Nodable = nullptr;

const Language* LanguageLibrary::GetNodable()
{
    if (LanguageLibrary::Nodable == nullptr)
    {
        LanguageLibrary::Nodable = new NodableLanguage();
    }
    return LanguageLibrary::Nodable;
}