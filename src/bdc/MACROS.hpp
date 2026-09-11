#pragma once

#ifdef BDC_ENABLE_LOGS
    #define BDC_LOG(...) printf(__VA_ARGS__)
    #define BDC_LOG_DEBUG(...) printf(__VA_ARGS__)
#else
    #define BDC_LOG(...)
    #define BDC_LOG_DEBUG(...)
#endif

