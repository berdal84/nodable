#pragma once

namespace ndbl::BuildInfo
{
    static const char* assets_dir        = NDBL_APP_ASSETS_DIR;
    static const char* version           = NDBL_APP_NAME NDBL_BUILD_REF;
    static const char* version_extended  = NDBL_APP_NAME NDBL_BUILD_REF " - Built " __DATE__ " at " __TIME__;
}
