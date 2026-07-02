#include "HookInit.hpp"
#include "VerificationBridge.h"
#include "NetworkGuard.hpp"
#include "OffsetConfig.h"
#include "XboxAuthFix.hpp"
#include "fishhook.h"
#include "api/APIScreen.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/cache.h>

static bool g_hooksInitialized = false;
static bool g_cppHooksActive = false;

static bool installObjCSwizzle(void) {
    void* handle = dlopen("/System/Library/Frameworks/UIKit.framework/UIKit", RTLD_NOLOAD);
    if (!handle) return false;
    dlclose(handle);
    return true;
}

static bool tryResolveCppHook(void) {
    OffsetConfig& cfg = OffsetConfig::shared();
    cfg.loadFromEnvironment();
    uint64_t addr = cfg.getOffset("ServerNetworkHandler_tick");
    if (addr == 0) return false;

    printf("[MCCL] Found ServerNetworkHandler_tick at 0x%llx\n", addr);
    void* target = (void*)addr;

    kern_return_t kr = vm_protect(mach_task_self(), (vm_address_t)target, 16, 0,
                                  VM_PROT_READ | VM_PROT_WRITE | VM_PROT_COPY);
    if (kr != KERN_SUCCESS) return false;

    int64_t offset = (int64_t)((uintptr_t)hookServerNetworkHandlerTick - (uintptr_t)target - 4);
    uint32_t branch = 0x14000000 | (((uint32_t)(offset >> 2)) & 0x03FFFFFF);
    uint32_t nop = 0xD503201F;
    memcpy(target, &branch, sizeof(branch));
    memcpy((void*)((uintptr_t)target + 4), &nop, sizeof(nop));

    kr = vm_protect(mach_task_self(), (vm_address_t)target, 16, 0,
                    VM_PROT_READ | VM_PROT_EXECUTE);
    sys_icache_invalidate((void*)target, 16);
    g_originalTick = (ServerNetworkHandler_tick_t)target;
    return true;
}

__attribute__((constructor(101)))
static void onLibraryLoad(void) {
    printf("[MCCL] Client Loader v1.0 initializing...\n");

    if (!verifyBaseGame()) return;

    installKeychainBypass();

    showAPISelector();

    bool objcOK = installObjCSwizzle();
    printf("[MCCL] Objective-C runtime %s\n", objcOK ? "available" : "not available");

    bool cppOK = tryResolveCppHook();
    if (cppOK) {
        printf("[MCCL] C++ hooks active (ServerNetworkHandler::tick)\n");
        g_cppHooksActive = true;
    } else {
        printf("[MCCL] C++ hooks not configured. Run Frida script to discover offsets.\n");
    }

    g_hooksInitialized = true;
    printf("[MCCL] Initialization complete\n");
}

void initializeClientMod(void) {
    printf("[MCCL] initializeClientMod() called — hooks %s, C++ %s\n",
           g_hooksInitialized ? "active" : "inactive",
           g_cppHooksActive ? "active" : "pending (run Frida script)");
}
