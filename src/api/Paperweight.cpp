#include "Paperweight.h"
#include "fishhook.h"
#include <cstdio>
#include <cstring>
#include <dlfcn.h>
#include <unordered_map>
#include <vector>
#include <functional>
#include <mutex>

namespace Paperweight {

static std::mutex g_mutex;
static std::unordered_map<std::string, std::function<void(const std::string&)>> g_commands;
static std::function<void(const std::string&, const std::string&)> g_chatCallback;
static std::vector<std::string> g_suppressedPatterns;
static float g_timerSpeed = 1.0f;
static float g_speedMultiplier = 1.0f;
static float g_flySpeed = 1.0f;
static bool g_noFall = false;
static bool g_autoJump = true;
static float g_stepHeight = 0.6f;
static bool g_bhop = false;
static bool g_espEnabled = false;
static float g_espRange = 100.0f;
static bool g_espShowPlayers = true;
static bool g_espShowMobs = false;
static bool g_espShowItems = false;
static float g_espColor[3] = {1.0f, 0.0f, 0.0f};
static bool g_espHealthBar = true;

// Hooked function types
typedef void* (*t_ClientInstance_getLocalPlayer)(void* this_);
typedef float (*t_Mob_getSpeed)(void* this_);
typedef void (*t_Mob_setSpeed)(void* this_, float);
typedef int (*t_Entity_getHealth)(void* this_);
typedef void (*t_Entity_setHealth)(void* this_, int);
typedef bool (*t_Player_isOnGround)(void* this_);
typedef bool (*t_Player_isSprinting)(void* this_);
typedef void (*t_Player_setSprinting)(void* this_, bool);
typedef void (*t_Level_sendMessage)(void* this_, const char* msg);
typedef void (*t_GameMode_destroyBlock)(void* this_, BlockPos*, int);

static t_ClientInstance_getLocalPlayer o_ClientInstance_getLocalPlayer = nullptr;
static t_Mob_getSpeed o_Mob_getSpeed = nullptr;
static t_Mob_setSpeed o_Mob_setSpeed = nullptr;
static t_Entity_getHealth o_Entity_getHealth = nullptr;
static t_Entity_setHealth o_Entity_setHealth = nullptr;
static t_Player_isOnGround o_Player_isOnGround = nullptr;
static t_Player_isSprinting o_Player_isSprinting = nullptr;
static t_Player_setSprinting o_Player_setSprinting = nullptr;
static t_Level_sendMessage o_Level_sendMessage = nullptr;
static t_GameMode_destroyBlock o_GameMode_destroyBlock = nullptr;

// ============ Player ============
Player* Player::getLocal() {
    void* client = dlsym(RTLD_DEFAULT, "?getClientInstance@@YAPEAVClientInstance@@XZ");
    if (!client) {
        void* handle = dlopen("libminecraftpe.dylib", RTLD_NOW);
        if (handle) {
            client = dlsym(handle, "?getClientInstance@@YAPEAVClientInstance@@XZ");
            if (!client) dlclose(handle);
        }
    }
    if (!client || !o_ClientInstance_getLocalPlayer) return nullptr;
    static Player instance;
    instance.entity_ = o_ClientInstance_getLocalPlayer((void*)client);
    return instance.entity_ ? &instance : nullptr;
}

Vec3 Player::getPosition() {
    Vec3 pos = {0, 0, 0};
    if (!entity_) return pos;
    float* base = (float*)entity_;
    pos.x = base[40];
    pos.y = base[41];
    pos.z = base[42];
    return pos;
}

void Player::setPosition(Vec3 pos) {
    if (!entity_) return;
    float* base = (float*)entity_;
    base[40] = pos.x;
    base[41] = pos.y;
    base[42] = pos.z;
}

void Player::setSpeed(float speed) {
    if (!entity_ || !o_Mob_setSpeed) return;
    o_Mob_setSpeed(entity_, speed);
}

void Player::setHealth(float health) {
    if (!entity_ || !o_Entity_setHealth) return;
    o_Entity_setHealth(entity_, (int)health);
}

float Player::getHealth() {
    if (!entity_ || !o_Entity_getHealth) return 0;
    return (float)o_Entity_getHealth(entity_);
}

int Player::getInventorySlot() {
    if (!entity_) return 0;
    return *(int*)((uintptr_t)entity_ + 1584);
}

void Player::sendMessage(const std::string& msg) {
    if (!entity_) return;
    void* level = *(void**)((uintptr_t)entity_ + 16);
    if (!level || !o_Level_sendMessage) return;
    o_Level_sendMessage(level, msg.c_str());
}

bool Player::isOnGround() {
    if (!entity_ || !o_Player_isOnGround) return false;
    return o_Player_isOnGround(entity_);
}

bool Player::isSprinting() {
    if (!entity_ || !o_Player_isSprinting) return false;
    return o_Player_isSprinting(entity_);
}

void Player::setSprinting(bool v) {
    if (!entity_ || !o_Player_setSprinting) return;
    o_Player_setSprinting(entity_, v);
}

bool Player::isFlying() {
    if (!entity_) return false;
    return *(bool*)((uintptr_t)entity_ + 4256);
}

void Player::setFlying(bool v) {
    if (!entity_) return;
    *(bool*)((uintptr_t)entity_ + 4256) = v;
}

// ============ Commands ============
void Commands::registerCmd(const std::string& name, const std::string& desc, std::function<void(const std::string&)> callback) {
    std::lock_guard<std::mutex> lock(g_mutex);
    g_commands[name] = callback;
    printf("[Paperweight] Registered command: .%s - %s\n", name.c_str(), desc.c_str());
}

void Commands::broadcast(const std::string& msg) {
    Player* p = Player::getLocal();
    if (p) p->sendMessage(msg);
}

// ============ HUD ============
void HUD::drawText(float x, float y, const std::string& text, float r, float g, float b, float a) {
    // Render will pick this up next frame
    struct HUDText { float x, y, r, g, b, a; std::string text; };
    static std::vector<HUDText> texts;
    texts.push_back({x, y, r, g, b, a, text});
}

void HUD::drawRect(float x, float y, float w, float h, float r, float g, float b, float a) {
    struct HUDRect { float x, y, w, h, r, g, b, a; };
    static std::vector<HUDRect> rects;
    rects.push_back({x, y, w, h, r, g, b, a});
}

void HUD::drawLine(float x1, float y1, float x2, float y2, float r, float g, float b, float a, float thickness) {
    struct HUDLine { float x1, y1, x2, y2, r, g, b, a, t; };
    static std::vector<HUDLine> lines;
    lines.push_back({x1, y1, x2, y2, r, g, b, a, thickness});
}

// ============ ESP ============
void ESP::setEnabled(bool v) { g_espEnabled = v; }
void ESP::setRange(float range) { g_espRange = range; }
void ESP::setShowPlayers(bool v) { g_espShowPlayers = v; }
void ESP::setShowMobs(bool v) { g_espShowMobs = v; }
void ESP::setShowItems(bool v) { g_espShowItems = v; }
void ESP::setBoxColor(float r, float g, float b) { g_espColor[0]=r; g_espColor[1]=g; g_espColor[2]=b; }
void ESP::setHealthBar(bool v) { g_espHealthBar = v; }

// ============ Chat ============
void Chat::onReceive(std::function<void(const std::string&, const std::string&)> callback) {
    std::lock_guard<std::mutex> lock(g_mutex);
    g_chatCallback = callback;
}

void Chat::send(const std::string& msg) {
    Player* p = Player::getLocal();
    if (p) p->sendMessage(msg);
}

void Chat::suppress(const std::string& pattern) {
    std::lock_guard<std::mutex> lock(g_mutex);
    g_suppressedPatterns.push_back(pattern);
}

// ============ Movement ============
void Movement::setSpeed(float speed) { g_speedMultiplier = speed; }
void Movement::setFlySpeed(float speed) { g_flySpeed = speed; }
void Movement::setNoFall(bool v) { g_noFall = v; }
void Movement::setAutoJump(bool v) { g_autoJump = v; }
void Movement::setStepHeight(float height) { g_stepHeight = height; }
void Movement::setBhop(bool v) { g_bhop = v; }

// ============ World ============
Vec3 World::getSpawnPoint() { return {0, 64, 0}; }
float World::getDayTime() { return 0; }
int World::getDifficulty() { return 2; }
bool World::isLoaded() { return true; }
int World::getTick() { return 0; }
std::string World::getWorldName() { return "Unknown"; }

// ============ Timer ============
void Timer::setSpeed(float multiplier) { g_timerSpeed = multiplier; }
float Timer::getSpeed() { return g_timerSpeed; }

// ============ Init ============
void init() {
    printf("[Paperweight] Lightweight API loaded\n");
    printf("[Paperweight] Commands prefix: .\n");
}

}
