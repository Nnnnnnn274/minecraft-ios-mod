#pragma once

#ifdef __APPLE__
#include <Security/Security.h>
#endif

void installKeychainBypass(void);
