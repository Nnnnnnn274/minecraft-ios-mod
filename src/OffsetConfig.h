#pragma once
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

class OffsetConfig {
public:
    static OffsetConfig& shared();

    bool loadFromFile(const char* path);
    bool loadFromEnvironment();

    uint64_t getOffset(const char* name) const;
    void setOffset(const char* name, uint64_t addr);

    bool hasLocalServerHook() const;

private:
    OffsetConfig();
    struct Entry {
        char name[128];
        uint64_t address;
        Entry* next;
    };
    Entry* head_;
    bool loaded_;
};
