#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #ifdef BUILDING_SHARED_LIB
        #define BRAW_BIND __declspec(dllexport)
    #else
        #define BRAW_BIND __declspec(dllimport)
    #endif
#else
    #define BRAW_BIND __attribute__((visibility("default")))
#endif