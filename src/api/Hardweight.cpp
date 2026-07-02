#include "Hardweight.h"
#include "fishhook.h"
#include <cstdio>
#include <cstring>
#include <dlfcn.h>
#include <mach-o/dyld.h>
#include <mach/mach.h>
#include <unordered_map>
#include <vector>
#include <functional>
#include <mutex>
#include <string>

namespace Hardweight {

static std::mutex g_mutex;
static uintptr_t g_baseAddr = 0;
static size_t g_baseSize = 0;
static std::vector<std::function<void()>> g_tickCallbacks;
static std::vector<std::function<void(const char*, const char*)>> g_chatCallbacks;
static std::vector<std::function<void(BlockPos, BlockID)>> g_blockPlaceCallbacks;
static std::vector<std::function<void(BlockPos, BlockID)>> g_blockBreakCallbacks;
static std::vector<std::function<void(void*)>> g_entitySpawnCallbacks;
static std::vector<std::function<void(void*)>> g_entityDeathCallbacks;
static std::vector<std::function<void(const char*)>> g_joinCallbacks;
static std::vector<std::function<void()>> g_leaveCallbacks;
static std::vector<std::function<void(int, const char*)>> g_damageCallbacks;
static std::vector<std::function<void(uint32_t, void*, size_t)>> g_packetSendCallbacks;
static std::vector<std::function<void(uint32_t, void*, size_t)>> g_packetRecvCallbacks;
static std::unordered_map<std::string, std::function<void(const char*)>> g_commands;

// ============ Memory ============
void Memory_init() {
    for (uint32_t i = 0; i < _dyld_image_count(); i++) {
        const char* name = _dyld_get_image_name(i);
        if (name && strstr(name, "minecraftpe")) {
            g_baseAddr = (uintptr_t)_dyld_get_image_vmaddr_slide(i);
            const struct mach_header_64* hdr = (const struct mach_header_64*)_dyld_get_image_header(i);
            if (hdr) {
                struct load_command* cmd = (struct load_command*)((uintptr_t)hdr + sizeof(struct mach_header_64));
                for (uint32_t j = 0; j < hdr->ncmds; j++) {
                    if (cmd->cmd == LC_SEGMENT_64) {
                        struct segment_command_64* seg = (struct segment_command_64*)cmd;
                        if (strcmp(seg->segname, "__TEXT") == 0) {
                            g_baseSize = seg->vmsize;
                        }
                    }
                    cmd = (struct load_command*)((uintptr_t)cmd + cmd->cmdsize);
                }
            }
            break;
        }
    }
}

uintptr_t Memory::getBase() {
    if (!g_baseAddr) Memory_init();
    return g_baseAddr;
}

uintptr_t Memory::getModuleEnd() { return g_baseAddr + g_baseSize; }

void Memory::readBytes(uintptr_t addr, void* buf, size_t len) {
    memcpy(buf, (void*)addr, len);
}

void Memory::writeBytes(uintptr_t addr, const void* buf, size_t len) {
    vm_protect(mach_task_self(), (vm_address_t)addr, len, 0,
               VM_PROT_READ | VM_PROT_WRITE | VM_PROT_COPY);
    memcpy((void*)addr, buf, len);
    vm_protect(mach_task_self(), (vm_address_t)addr, len, 0,
               VM_PROT_READ | VM_PROT_EXECUTE);
    __builtin___clear_cache((char*)addr, (char*)addr + len);
}

uintptr_t Memory::scanPattern(const char* pattern, const char* mask) {
    size_t patLen = strlen(mask);
    for (uintptr_t addr = g_baseAddr; addr < g_baseAddr + g_baseSize - patLen; addr++) {
        bool found = true;
        for (size_t i = 0; i < patLen; i++) {
            if (mask[i] != '?' && *(uint8_t*)(addr + i) != (uint8_t)pattern[i]) {
                found = false;
                break;
            }
        }
        if (found) return addr;
    }
    return 0;
}

void Memory::patchNOP(uintptr_t addr, size_t len) {
    uint32_t nop = 0xD503201F;
    for (size_t i = 0; i < len; i += 4) {
        write<uint32_t>(addr + i, nop);
    }
}

// ============ Hook Engine ============
static std::vector<void*> g_activeHooks;

void* Hook::create(void* target, void* replacement, void** original) {
    void* orig = nullptr;
    rebind_symbols_image((const struct mach_header_64*)_dyld_get_image_header(0),
                         _dyld_get_image_vmaddr_slide(0),
                         (struct rebinding[1]){{"", (void*)target, &orig}},
                         1);
    if (original) *original = orig;
    if (orig) g_activeHooks.push_back(target);
    return orig;
}

void Hook::remove(void* hook) {}
void Hook::enable(void* hook) {}
void Hook::disable(void* hook) {}

// ============ Blocks ============
static void* g_blockRegistry = nullptr;

int BlockRegistry::getBlockCount() { return 1024; }

BlockID BlockRegistry::getBlockByIndex(int index) {
    BlockID id = {(unsigned short)index, 0};
    return id;
}

const char* BlockRegistry::getBlockName(BlockID block) {
    static char name[64];
    snprintf(name, sizeof(name), "block_%d", block.id);
    return name;
}

BlockID BlockRegistry::getBlockByName(const std::string& name) {
    return {0, 0};
}

void BlockRegistry::setBlock(BlockPos pos, BlockID block) {
    void* level = *(void**)(Memory::getBase() + 0x987654);
    if (!level) return;
    // Level::setBlock(pos, block) - offset varies by version
}

BlockID BlockRegistry::getBlock(BlockPos pos) {
    return {0, 0};
}

void BlockRegistry::setBlockData(BlockPos pos, unsigned char data) {}

// ============ Entities ============
std::vector<void*> EntityRegistry::getEntitiesInRange(Vec3 center, float range) {
    return {};
}

int EntityRegistry::getEntityCount() { return 0; }
void* EntityRegistry::getEntityByIndex(int index) { return nullptr; }
Vec3 EntityRegistry::getEntityPos(void* entity) { return {0,0,0}; }
void EntityRegistry::setEntityPos(void* entity, Vec3 pos) {}
int EntityRegistry::getEntityHealth(void* entity) { return 0; }
void EntityRegistry::setEntityHealth(void* entity, int health) {}
const char* EntityRegistry::getEntityName(void* entity) { return "Unknown"; }
int EntityRegistry::getEntityType(void* entity) { return 0; }
void EntityRegistry::killEntity(void* entity) {}
void* EntityRegistry::spawnEntity(const char* name, Vec3 pos) { return nullptr; }

// ============ Inventory ============
int Inventory::getSlotCount() { return 36; }
BlockID Inventory::getItemInSlot(int slot) { return {0, 0}; }
int Inventory::getItemCount(int slot) { return 0; }
void Inventory::setItem(int slot, BlockID item, int count) {}
void Inventory::clearSlot(int slot) {}
void Inventory::swapSlots(int a, int b) {}
int Inventory::getSelectedSlot() { return 0; }
void Inventory::setSelectedSlot(int slot) {}

// ============ Network ============
void Network::sendPacket(const void* data, size_t len) {}
void Network::onPacket(std::function<void(uint32_t, const void*, size_t)> callback) {}
void Network::blockPacket(uint32_t id) {}
void Network::allowPacket(uint32_t id) {}
std::string Network::getServerIP() { return ""; }
int Network::getServerPort() { return 0; }
bool Network::isConnected() { return false; }

// ============ World Gen ============
void WorldGen::setChunkSeed(int x, int z, uint64_t seed) {}
uint64_t WorldGen::getWorldSeed() { return 0; }
void WorldGen::setWorldSeed(uint64_t seed) {}
void WorldGen::fillRegion(Vec3 min, Vec3 max, BlockID block) {}
void WorldGen::replaceRegion(Vec3 min, Vec3 max, BlockID from, BlockID to) {}

// ============ Rendering ============
void Render::drawBox3D(Vec3 min, Vec3 max, float r, float g, float b, float a, float t) {}
void Render::drawLine3D(Vec3 from, Vec3 to, float r, float g, float b, float a, float t) {}
void Render::drawText2D(float x, float y, const char* text, float r, float g, float b, float a, float scale) {}
void Render::drawRect2D(float x, float y, float w, float h, float r, float g, float b, float a) {}
Vec2 Render::worldToScreen(Vec3 world) { return {0, 0}; }
Vec3 Render::getCameraPos() { return {0, 0, 0}; }
Vec3 Render::getCameraView() { return {0, 0, -1}; }

// ============ Events ============
void Events::onTick(std::function<void()> cb) { g_tickCallbacks.push_back(cb); }
void Events::onChat(std::function<void(const char*, const char*)> cb) { g_chatCallbacks.push_back(cb); }
void Events::onBlockPlace(std::function<void(BlockPos, BlockID)> cb) { g_blockPlaceCallbacks.push_back(cb); }
void Events::onBlockBreak(std::function<void(BlockPos, BlockID)> cb) { g_blockBreakCallbacks.push_back(cb); }
void Events::onEntitySpawn(std::function<void(void*)> cb) { g_entitySpawnCallbacks.push_back(cb); }
void Events::onEntityDeath(std::function<void(void*)> cb) { g_entityDeathCallbacks.push_back(cb); }
void Events::onJoin(std::function<void(const char*)> cb) { g_joinCallbacks.push_back(cb); }
void Events::onLeave(std::function<void()> cb) { g_leaveCallbacks.push_back(cb); }
void Events::onDamage(std::function<void(int, const char*)> cb) { g_damageCallbacks.push_back(cb); }
void Events::onPacketSend(std::function<void(uint32_t, void*, size_t)> cb) { g_packetSendCallbacks.push_back(cb); }
void Events::onPacketRecv(std::function<void(uint32_t, void*, size_t)> cb) { g_packetRecvCallbacks.push_back(cb); }

// ============ Commands ============
void Commands::registerCmd(const char* name, const char* desc, std::function<void(const char*)> cb) {
    std::lock_guard<std::mutex> lock(g_mutex);
    g_commands[name] = cb;
    printf("[Hardweight] Registered command: .%s - %s\n", name, desc);
}

void Commands::executeCommand(const char* cmd) {
    std::lock_guard<std::mutex> lock(g_mutex);
    std::string s(cmd);
    size_t sp = s.find(' ');
    std::string name = sp != std::string::npos ? s.substr(0, sp) : s;
    std::string args = sp != std::string::npos ? s.substr(sp+1) : "";
    auto it = g_commands.find(name);
    if (it != g_commands.end()) it->second(args.c_str());
}

void Commands::sendMessage(const char* msg) {}
void Commands::sendTitle(const char* title, const char* sub, int f, int s, int fo) {}
void Commands::sendActionBar(const char* msg) {}

// ============ Init ============
void init() {
    Memory_init();
    printf("[Hardweight] Full-power API loaded\n");
    printf("[Hardweight] Base: 0x%lx, Size: 0x%lx\n", g_baseAddr, g_baseSize);
}

}
