#ifdef WIN32
#include <windows.h>
#elif __APPLE__
#include <dlfcn.h>
#include <mach-o/dyld.h>
#include <sys/syslimits.h>
#include <stdlib.h>
#else
#include <dlfcn.h>
#include <linux/limits.h>
#include <cstring>
#endif

#include <vector>
#include <string>

#include <audience.h>

#include "../common/trace.h"
#include "nucleus.h"
#include "whereami.h"

#if defined(WIN32)
#define PATH_SEPARATOR "\\"
#else
#define PATH_SEPARATOR "/"
#endif

std::string whereami();

nucleus_init_t nucleus_init = nullptr;
nucleus_window_create_t nucleus_window_create = nullptr;
nucleus_window_destroy_t nucleus_window_destroy = nullptr;
nucleus_loop_t nucleus_loop = nullptr;

bool audience_is_initialized()
{
  return nucleus_init != nullptr && nucleus_window_create != nullptr && nucleus_window_destroy != nullptr && nucleus_loop != nullptr;
}

bool audience_init()
{
  if (audience_is_initialized())
  {
    return true;
  }

  std::vector<std::string> dylibs{
#ifdef WIN32
      "audience_windows_edge.dll",
      "audience_windows_ie11.dll",
#elif __APPLE__
      "libaudience_macos_webkit.dylib",
#else
      "libaudience_unix_webkit.so",
#endif
  };

  for (auto dylib : dylibs)
  {
    auto dylib_abs = whereami() + PATH_SEPARATOR + dylib;
    TRACEA(info, "trying to load library from path " << dylib_abs);
#ifdef WIN32
    auto dlh = LoadLibraryA(dylib_abs.c_str());
#else
    auto dlh = dlopen(dylib_abs.c_str(), RTLD_LAZY | RTLD_LOCAL);
#endif

    if (dlh != nullptr)
    {
#ifdef WIN32
#define LookupFunction GetProcAddress
#else
#define LookupFunction dlsym
#endif

      nucleus_init = (nucleus_init_t)LookupFunction(dlh, "audience_init");
      nucleus_window_create = (nucleus_window_create_t)LookupFunction(dlh, "audience_window_create");
      nucleus_window_destroy = (nucleus_window_destroy_t)LookupFunction(dlh, "audience_window_destroy");
      nucleus_loop = (nucleus_loop_t)LookupFunction(dlh, "audience_loop");

      if (!audience_is_initialized())
      {
        TRACEA(info, "could not find function pointer in library " << dylib);
      }

      if (audience_is_initialized() && nucleus_init())
      {
        TRACEA(info, "library " << dylib << " loaded successfully");
        return true;
      }
      else
      {
        TRACEA(info, "could not initialize library " << dylib);
      }

      nucleus_init = nullptr;
      nucleus_window_create = nullptr;
      nucleus_window_destroy = nullptr;
      nucleus_loop = nullptr;

#ifdef WIN32
      FreeLibrary(dlh);
#else
      dlclose(dlh);
#endif
    }
    else
    {
      TRACEA(info, "could not load library " << dylib);
#ifdef WIN32
      TRACEW(info, GetLastErrorString().c_str());
#endif
    }
  }

  return false;
}

void *audience_window_create(const wchar_t *const title, const wchar_t *const url)
{
  if (!audience_is_initialized())
  {
    return nullptr;
  }
  return nucleus_window_create(title, url);
}

void audience_window_destroy(void *window)
{
  if (!audience_is_initialized())
  {
    return;
  }
  nucleus_window_destroy(window);
}

void audience_loop()
{
  if (!audience_is_initialized())
  {
    return;
  }
  nucleus_loop();
}

std::string whereami()
{
  auto length = wai_getExecutablePath(nullptr, 0, nullptr);
  if (length == -1)
  {
    TRACEA(warning, "could not retrieve path of executable");
    return "";
  }
  std::vector<char> buffer(length + 1, 0);
  int dir_length = 0;
  if (wai_getExecutablePath(&buffer[0], length, &dir_length) == -1)
  {
    TRACEA(warning, "could not retrieve path of executable");
    return "";
  }
  buffer[dir_length] = 0;
  std::string path(&buffer[0]);
  TRACEA(info, "executable directory found: " << path);
  return path;
}
