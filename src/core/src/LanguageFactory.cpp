#include <nodable/LanguageFactory.h>
#include <nodable/LanguageNodable.h>

using namespace Nodable;

const Language* LanguageFactory::Nodable = nullptr;

const Language* LanguageFactory::GetNodable()
{
    if (LanguageFactory::Nodable == nullptr)
    {
        LanguageFactory::Nodable = new LanguageNodable();
    }
    return LanguageFactory::Nodable;
}