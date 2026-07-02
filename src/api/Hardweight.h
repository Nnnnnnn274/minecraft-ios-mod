#pragma once
#include <string>
#include <functional>
#include <vector>
#include <cstdint>

namespace Hardweight {

struct Vec3 { float x, y, z; };
struct Vec2 { float x, y; };
struct BlockPos { int x, y, z; };
struct BlockID { unsigned short id; unsigned char data; };

// ============ Memory ============
class Memory {
public:
    static uintptr_t getBase();
    static uintptr_t getModuleEnd();
    template<typename T> static T read(uintptr_t addr) { return *(T*)addr; }
    template<typename T> static void write(uintptr_t addr, T val) { *(T*)addr = val; }
    static void readBytes(uintptr_t addr, void* buf, size_t len);
    static void writeBytes(uintptr_t addr, const void* buf, size_t len);
    static uintptr_t scanPattern(const char* pattern, const char* mask);
    static void patchNOP(uintptr_t addr, size_t len);
};

// ============ Hook Engine ============
class Hook {
public:
    static void* create(void* target, void* replacement, void** original);
    static void remove(void* hook);
    static void enable(void* hook);
    static void disable(void* hook);
};

// ============ Blocks ============
class BlockRegistry {
public:
    static int getBlockCount();
    static BlockID getBlockByIndex(int index);
    static const char* getBlockName(BlockID block);
    static BlockID getBlockByName(const std::string& name);
    static void setBlock(BlockPos pos, BlockID block);
    static BlockID getBlock(BlockPos pos);
    static void setBlockData(BlockPos pos, unsigned char data);
};

// ============ Entities ============
class EntityRegistry {
public:
    static int getEntityCount();
    static void* getEntityByIndex(int index);
    static Vec3 getEntityPos(void* entity);
    static void setEntityPos(void* entity, Vec3 pos);
    static int getEntityHealth(void* entity);
    static void setEntityHealth(void* entity, int health);
    static const char* getEntityName(void* entity);
    static int getEntityType(void* entity);
    static void killEntity(void* entity);
    static void* spawnEntity(const char* name, Vec3 pos);
    static std::vector<void*> getEntitiesInRange(Vec3 center, float range);
};

// ============ Inventory ============
class Inventory {
public:
    static int getSlotCount();
    static BlockID getItemInSlot(int slot);
    static int getItemCount(int slot);
    static void setItem(int slot, BlockID item, int count);
    static void clearSlot(int slot);
    static void swapSlots(int a, int b);
    static int getSelectedSlot();
    static void setSelectedSlot(int slot);
};

// ============ Network ============
class Network {
public:
    static void sendPacket(const void* data, size_t len);
    static void onPacket(std::function<void(uint32_t id, const void* data, size_t len)> callback);
    static void blockPacket(uint32_t id);
    static void allowPacket(uint32_t id);
    static std::string getServerIP();
    static int getServerPort();
    static bool isConnected();
};

// ============ World Gen ============
class WorldGen {
public:
    static void setChunkSeed(int x, int z, uint64_t seed);
    static uint64_t getWorldSeed();
    static void setWorldSeed(uint64_t seed);
    static void fillRegion(Vec3 min, Vec3 max, BlockID block);
    static void replaceRegion(Vec3 min, Vec3 max, BlockID from, BlockID to);
};

// ============ Rendering ============
class Render {
public:
    static void drawBox3D(Vec3 min, Vec3 max, float r, float g, float b, float a, float thickness);
    static void drawLine3D(Vec3 from, Vec3 to, float r, float g, float b, float a, float thickness);
    static void drawText2D(float x, float y, const char* text, float r, float g, float b, float a, float scale);
    static void drawRect2D(float x, float y, float w, float h, float r, float g, float b, float a);
    static Vec2 worldToScreen(Vec3 world);
    static Vec3 getCameraPos();
    static Vec3 getCameraView();
};

// ============ Events ============
class Events {
public:
    static void onTick(std::function<void()> callback);
    static void onChat(std::function<void(const char* sender, const char* msg)> callback);
    static void onBlockPlace(std::function<void(BlockPos pos, BlockID block)> callback);
    static void onBlockBreak(std::function<void(BlockPos pos, BlockID block)> callback);
    static void onEntitySpawn(std::function<void(void* entity)> callback);
    static void onEntityDeath(std::function<void(void* entity)> callback);
    static void onJoin(std::function<void(const char* serverIP)> callback);
    static void onLeave(std::function<void()> callback);
    static void onDamage(std::function<void(int amount, const char* source)> callback);
    static void onPacketSend(std::function<void(uint32_t id, void* data, size_t len)> callback);
    static void onPacketRecv(std::function<void(uint32_t id, void* data, size_t len)> callback);
};

// ============ Commands ============
class Commands {
public:
    static void registerCmd(const char* name, const char* desc, std::function<void(const char* args)> callback);
    static void executeCommand(const char* cmd);
    static void sendMessage(const char* msg);
    static void sendTitle(const char* title, const char* subtitle, int fadeIn, int stay, int fadeOut);
    static void sendActionBar(const char* msg);
};

void init();

}
