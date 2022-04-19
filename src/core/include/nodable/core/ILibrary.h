#pragma once

namespace Nodable
{
    class ILanguage;

    class ILibrary
    {
    public:
        virtual ~ILibrary() = default;
        virtual void bind_to_language(ILanguage* _language)const = 0;
    };
}
