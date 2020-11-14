#pragma once

/**
    >> DO NOT EDIT Config.h, edit Config.h.in (source file) <<

    Config.h.in will be configured by cmake, check CMakelists.txt
*/

#define NODABLE_ASSETS_DIR "assets"
#define NODABLE_VERSION "0.7.0"

#ifdef _DEBUG
#define NODABLE_VERSION_EXTENDED NODABLE_VERSION " (Debug) Build " __DATE__ " at " __TIME__
#else
#define NODABLE_VERSION_EXTENDED NODABLE_VERSION " (Release) Build " __DATE__ " at " __TIME__
#endif

