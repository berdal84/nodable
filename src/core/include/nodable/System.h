#pragma once
#include <string>

namespace Nodable::core
{
    /**
     * Static library cross platform to deal with system
     */
    namespace System
    {
        /**
        * Open an URL asynchronously.
        * @return the system result code as a future.
        */
        void OpenURL(std::string /* URL */);
    }
};

