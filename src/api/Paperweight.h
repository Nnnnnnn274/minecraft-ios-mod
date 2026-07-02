#pragma once
#include <string>
#include <functional>
#include <vector>

namespace Paperweight {

struct Vec3 { float x, y, z; };

class Player {
public:
    static Player* getLocal();
    Vec3 getPosition();
    void* entity_ = nullptr;
    void setPosition(Vec3 pos);
    void setSpeed(float speed);
    void setHealth(float health);
    float getHealth();
    int getInventorySlot();
    void sendMessage(const std::string& msg);
    bool isOnGround();
    bool isSprinting();
    void setSprinting(bool v);
    void setFlying(bool v);
    bool isFlying();
};

class Commands {
public:
    static void registerCmd(const std::string& name, const std::string& desc, std::function<void(const std::string&)> callback);
    static void broadcast(const std::string& msg);
};

class HUD {
public:
    static void drawText(float x, float y, const std::string& text, float r, float g, float b, float a);
    static void drawRect(float x, float y, float w, float h, float r, float g, float b, float a);
    static void drawLine(float x1, float y1, float x2, float y2, float r, float g, float b, float a, float thickness);
};

class ESP {
public:
    static void setEnabled(bool v);
    static void setRange(float range);
    static void setShowPlayers(bool v);
    static void setShowMobs(bool v);
    static void setShowItems(bool v);
    static void setBoxColor(float r, float g, float b);
    static void setHealthBar(bool v);
};

class Chat {
public:
    static void onReceive(std::function<void(const std::string& sender, const std::string& msg)> callback);
    static void send(const std::string& msg);
    static void suppress(const std::string& pattern);
};

class Movement {
public:
    static void setSpeed(float speed);
    static void setFlySpeed(float speed);
    static void setNoFall(bool v);
    static void setAutoJump(bool v);
    static void setStepHeight(float height);
    static void setBhop(bool v);
};

class World {
public:
    static Vec3 getSpawnPoint();
    static float getDayTime();
    static int getDifficulty();
    static bool isLoaded();
    static int getTick();
    static std::string getWorldName();
};

class Timer {
public:
    static void setSpeed(float multiplier);
    static float getSpeed();
};

void init();

}
