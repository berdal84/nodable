#pragma once
#include <nodable/Serializer.h>
#include <nodable/NodableLanguage.h>

namespace Nodable::core
{
    class NodableSerializer: public Serializer {
    public:
        NodableSerializer(const NodableLanguage* _language):Serializer(_language){}
        ~NodableSerializer() = default;
    };
}
