#pragma once
#include <Serializer.h>
#include "NodableLanguage.h"

namespace Nodable
{
    class NodableSerializer: public Serializer {
    public:
        NodableSerializer(const NodableLanguage* _language):Serializer(_language){}
        ~NodableSerializer() = default;
    };
}
