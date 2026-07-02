#include "XboxAuthFix.hpp"
#include "fishhook.h"
#include <Foundation/Foundation.h>
#include <stdio.h>
#include <string.h>

static CFTypeRef (*original_SecItemAdd)(CFDictionaryRef, CFTypeRef *) = NULL;
static OSStatus (*original_SecItemCopyMatching)(CFDictionaryRef, CFTypeRef *) = NULL;
static OSStatus (*original_SecItemUpdate)(CFDictionaryRef, CFDictionaryRef) = NULL;
static OSStatus (*original_SecItemDelete)(CFDictionaryRef) = NULL;

static NSString* getKeychainPrefix(void) {
    return @"com.xbox.auth";
}

static NSString* tokenFilePath(NSString* service, NSString* account) {
    NSString* dir = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) firstObject];
    NSString* safeService = [service stringByReplacingOccurrencesOfString:@"/" withString:@"_"];
    NSString* safeAccount = [account stringByReplacingOccurrencesOfString:@"/" withString:@"_"];
    return [NSString stringWithFormat:@"%@/keychain/%@_%@.dat", dir, safeService, safeAccount];
}

static void ensureKeychainDir(void) {
    NSString* dir = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) firstObject];
    NSString* kcDir = [dir stringByAppendingPathComponent:@"keychain"];
    [[NSFileManager defaultManager] createDirectoryAtPath:kcDir
                              withIntermediateDirectories:YES
                                               attributes:nil
                                                    error:NULL];
}

static void saveToLocalStorage(NSDictionary* query, NSData* data) {
    ensureKeychainDir();
    NSString* service = query[(__bridge id)kSecAttrService];
    NSString* account = query[(__bridge id)kSecAttrAccount];
    if (!service || !account) return;
    NSString* path = tokenFilePath(service, account);
    [data writeToFile:path atomically:YES];
}

static NSData* loadFromLocalStorage(NSDictionary* query) {
    NSString* service = query[(__bridge id)kSecAttrService];
    NSString* account = query[(__bridge id)kSecAttrAccount];
    if (!service || !account) return nil;
    NSString* path = tokenFilePath(service, account);
    return [NSData dataWithContentsOfFile:path];
}

static void deleteFromLocalStorage(NSDictionary* query) {
    NSString* service = query[(__bridge id)kSecAttrService];
    NSString* account = query[(__bridge id)kSecAttrAccount];
    if (!service || !account) return;
    NSString* path = tokenFilePath(service, account);
    [[NSFileManager defaultManager] removeItemAtPath:path error:NULL];
}

static CFTypeRef replacement_SecItemAdd(CFDictionaryRef attributes, CFTypeRef *result) {
    if (attributes) {
        NSDictionary* query = (__bridge NSDictionary*)attributes;
        NSData* data = query[(__bridge id)kSecValueData];
        if (data) {
            saveToLocalStorage(query, data);
            if (result) *result = NULL;
            return errSecSuccess;
        }
    }
    if (original_SecItemAdd) return original_SecItemAdd(attributes, result);
    return errSecParam;
}

static OSStatus replacement_SecItemCopyMatching(CFDictionaryRef query, CFTypeRef *result) {
    if (query && result) {
        NSDictionary* q = (__bridge NSDictionary*)query;
        NSData* data = loadFromLocalStorage(q);
        if (data) {
            *result = (__bridge_retained CFTypeRef)[data copy];
            return errSecSuccess;
        }
    }
    if (original_SecItemCopyMatching) return original_SecItemCopyMatching(query, result);
    return errSecItemNotFound;
}

static OSStatus replacement_SecItemUpdate(CFDictionaryRef query, CFDictionaryRef attributesToUpdate) {
    if (query && attributesToUpdate) {
        NSDictionary* q = (__bridge NSDictionary*)query;
        NSDictionary* update = (__bridge NSDictionary*)attributesToUpdate;
        NSData* data = update[(__bridge id)kSecValueData];
        if (data) {
            saveToLocalStorage(q, data);
            return errSecSuccess;
        }
    }
    if (original_SecItemUpdate) return original_SecItemUpdate(query, attributesToUpdate);
    return errSecParam;
}

static OSStatus replacement_SecItemDelete(CFDictionaryRef query) {
    if (query) {
        NSDictionary* q = (__bridge NSDictionary*)query;
        deleteFromLocalStorage(q);
        return errSecSuccess;
    }
    if (original_SecItemDelete) return original_SecItemDelete(query);
    return errSecParam;
}

void installKeychainBypass(void) {
    struct rebinding rebindings[] = {
        {"SecItemAdd", (void*)replacement_SecItemAdd, (void**)&original_SecItemAdd},
        {"SecItemCopyMatching", (void*)replacement_SecItemCopyMatching, (void**)&original_SecItemCopyMatching},
        {"SecItemUpdate", (void*)replacement_SecItemUpdate, (void**)&original_SecItemUpdate},
        {"SecItemDelete", (void*)replacement_SecItemDelete, (void**)&original_SecItemDelete},
    };
    rebind_symbols(rebindings, 4);
    printf("[MCCL] Keychain bypass installed (SecItem hooks active)\n");
}
