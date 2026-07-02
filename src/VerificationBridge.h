/*
 *  VerificationBridge.h — Minecraft Client Loader
 *  ===============================================
 *  Shared C-callable declaration for the anti-piracy verification function.
 *  This header is safe to include from both C++ (.cpp) and Objective-C++ (.mm)
 *  translation units.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

bool verifyBaseGame(void);

#ifdef __cplusplus
}
#endif
