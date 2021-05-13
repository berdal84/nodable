#pragma once
#include "Language.h"

namespace Nodable
{
    /**
     * Static class to get a specific Language
     */
    class LanguageFactory
    {
    public:
        /**
          * Get the Nodable Language (Singleton)
          */
        static const Language* GetNodable();

        /*
          * Get the XXX Language (Singleton)
          */
        // static const Language* XXX();
    private:
        static const Language* Nodable;
    };
}


