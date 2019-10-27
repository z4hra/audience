#pragma once

#if AUDIENCE_STATIC_LIBRARY

#define AUDIENCE_API

#else

#ifdef WIN32
#define AUDIENCE_API __declspec(dllexport)
#else
#define AUDIENCE_API
#endif

#endif

#include <audience_api.h>
