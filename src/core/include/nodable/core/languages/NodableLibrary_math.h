#pragma once

#include <nodable/core/ILibrary.h>

namespace Nodable
{
    class NodableLibrary_math : public ILibrary
    {
    public:
        void bind_to_language(ILanguage* _language)const override;
    };
}