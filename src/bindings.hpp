#pragma once

#if defined(_MSC_VER)
#define DllExport __declspec (dllexport)
#endif
#if defined(__GNUC__)
#define DllExport __attribute__((visibility("default")))
#endif