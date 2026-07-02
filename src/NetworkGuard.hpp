/*
 *  NetworkGuard.hpp — Minecraft Client Loader
 *  ============================================
 *  Component 2: Network-Aware Hybrid Guard Logic
 *
 *  Purpose:
 *    Provides a global runtime state tracker (g_isPublicServer) and a
 *    hookable tick function for ServerNetworkHandler. When the player is
 *    on a public server/realms, all mods are skipped to keep gameplay
 *    vanilla. In local/P2P sessions, executeServerSideMods() is called
 *    to apply custom modifications.
 */

#pragma once
#include <cstdint>

/*
 *  Global state: tracks whether the current connection is a public server.
 *  Set to true by default (safe default). The mod or network layer should
 *  set this to false when a local host or friend session is detected.
 */
extern bool g_isPublicServer;

/*
 *  Typedef for the original ServerNetworkHandler::tick function signature.
 *  This matches the game engine's expected ABI for the tick method.
 *    self       - pointer to the ServerNetworkHandler instance (this)
 *    deltaTime  - frame delta time in seconds from the engine
 */
typedef void (*ServerNetworkHandler_tick_t)(void* self, float deltaTime);

/*
 *  Saved pointer to the original tick function, populated after the
 *  inline hook is installed so we can forward calls transparently.
 */
extern ServerNetworkHandler_tick_t g_originalTick;

/*
 *  Replacement function installed as a hook over ServerNetworkHandler::tick.
 *  Always calls through to the original first, then applies the public
 *  server guard before running any mod logic.
 */
void hookServerNetworkHandlerTick(void* self, float deltaTime);

/*
 *  Placeholder execution pipeline for server-side modifications.
 *  Called only when g_isPublicServer == false. This is the single
 *  entry point for all custom gameplay logic.
 */
void executeServerSideMods(void);
