#pragma once

#include <audience_details.h>

#ifdef __cplusplus
extern "C"
{
#endif

  bool audience_init();
  bool audience_is_initialized();
  void *audience_window_create(const AudienceWindowDetails *details);
  void audience_window_destroy(void *handle);
  void audience_loop();

#ifdef __cplusplus
}
#endif
