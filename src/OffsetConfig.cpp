#include "OffsetConfig.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

OffsetConfig& OffsetConfig::shared() {
    static OffsetConfig instance;
    return instance;
}

OffsetConfig::OffsetConfig() : head_(nullptr), loaded_(false) {}

bool OffsetConfig::loadFromFile(const char* path) {
    FILE* f = fopen(path, "r");
    if (!f) return false;
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        char* nl = strchr(line, '\n'); if (nl) *nl = '\0';
        char* cr = strchr(line, '\r'); if (cr) *cr = '\0';
        if (line[0] == '\0' || line[0] == '#') continue;
        char name[128];
        uint64_t addr;
        if (sscanf(line, "%127s 0x%llx", name, &addr) >= 2) {
            setOffset(name, addr);
        } else if (sscanf(line, "%127s %llu", name, &addr) >= 2) {
            setOffset(name, addr);
        }
    }
    fclose(f);
    loaded_ = true;
    printf("[MCCL] Loaded offsets from: %s\n", path);
    return true;
}

bool OffsetConfig::loadFromEnvironment() {
    const char* path = getenv("MCCL_OFFSETS");
    if (path) return loadFromFile(path);
    path = "/var/mobile/Documents/mccl_offsets.txt";
    if (loadFromFile(path)) return true;
    path = "mccl_offsets.txt";
    return loadFromFile(path);
}

uint64_t OffsetConfig::getOffset(const char* name) const {
    Entry* cur = head_;
    while (cur) {
        if (strcmp(cur->name, name) == 0) return cur->address;
        cur = cur->next;
    }
    return 0;
}

void OffsetConfig::setOffset(const char* name, uint64_t addr) {
    Entry* cur = head_;
    while (cur) {
        if (strcmp(cur->name, name) == 0) {
            cur->address = addr;
            return;
        }
        cur = cur->next;
    }
    Entry* e = (Entry*)malloc(sizeof(Entry));
    strncpy(e->name, name, sizeof(e->name) - 1);
    e->name[sizeof(e->name) - 1] = '\0';
    e->address = addr;
    e->next = head_;
    head_ = e;
}

bool OffsetConfig::hasLocalServerHook() const {
    return getOffset("ServerNetworkHandler_tick") != 0;
}
