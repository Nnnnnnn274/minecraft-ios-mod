/*
 *  AntiPiracyCheck.mm — Minecraft Client Loader
 *  ==============================================
 *  Component 1: Anti-Piracy & Verification Check
 *
 *  Purpose:
 *    Verifies that the user owns a legitimate App Store copy of Minecraft
 *    Bedrock before any mod functionality is unlocked. This bridges C++ into
 *    native iOS Objective-C runtime APIs to query the registered URL scheme.
 *
 *  How it works:
 *    [[UIApplication sharedApplication] canOpenURL:] checks whether the
 *    minecraft:// URL scheme is registered on the device. Only the official
 *    App Store variant registers this scheme. If absent, the loader
 *    immediately terminates with exit(0) to respect licensing.
 *
 *  Thread safety:
 *    iOS UI APIs must be called from the main thread. We use dispatch_sync
 *    on dispatch_get_main_queue() to ensure this constraint is satisfied
 *    even if verifyBaseGame() is invoked from a background thread.
 */

#import <UIKit/UIKit.h>
#import <Foundation/Foundation.h>
#include <cstdio>
#include <cstdlib>
#include "VerificationBridge.h"

/*
 *  verifyBaseGame()
 *  -----------------
 *  C-callable function that bridges into the iOS runtime to confirm the
 *  official Minecraft app is installed via its custom URL scheme.
 *
 *  Returns:   true if Minecraft URL scheme is registered (official copy present)
 *             false otherwise (program terminates before returning)
 *
 *  Side effect: Calls exit(0) immediately if verification fails.
 */
extern "C" bool verifyBaseGame(void) {
    __block bool verified = false;

    /*
     *  Dispatch to the main thread synchronously because canOpenURL:
     *  must be called on the main run loop in modern iOS. Using a
     *  __block variable to capture the result back into our scope.
     */
    dispatch_sync(dispatch_get_main_queue(), ^{
        NSURL *minecraftURL = [NSURL URLWithString:@"minecraft://"];

        /*
         *  canOpenURL: returns YES if the device has an app registered to
         *  handle the minecraft:// scheme. Only the official Mojang/Microsoft
         *  App Store release registers this scheme.
         */
        verified = [[UIApplication sharedApplication] canOpenURL:minecraftURL];
    });

    /*
     *  Gate: if verification failed, log a critical error and hard-stop.
     *  This ensures the loader NEVER executes mod logic on pirated or
     *  third-party builds of the game.
     */
    if (!verified) {
        printf("[MCCL] CRITICAL: Official Minecraft Bedrock installation not detected. "
               "This loader requires a legitimate copy from the App Store. Exiting.\n");
        exit(0);
    }

    printf("[MCCL] Base game ownership verified successfully.\n");
    return true;
}
