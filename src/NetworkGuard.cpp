/*
 *  NetworkGuard.cpp — Minecraft Client Loader
 *  ===========================================
 *  Component 2: Network-Aware Hybrid Guard Logic (Implementation)
 *
 *  Design rationale:
 *    The tick hook is the heartbeat of the game loop. By intercepting it
 *    we get a per-frame callback that lets us conditionally inject mod
 *    behavior. The guard ensures zero mod code runs on public servers,
 *    keeping the user safe from anti-cheat flags and preserving vanilla
 *    play for other players.
 */

#include "NetworkGuard.hpp"
#include <cstdio>

/*
 *  Global reference tracking whether the client is connected to a public
 *  server (realms, external servers) or a local/P2P session. Initialized
 *  to true as a safe default — mods are disabled until explicitly allowed.
 */
bool g_isPublicServer = true;

/*
 *  Function pointer storage for the original ServerNetworkHandler::tick.
 *  Set by the hook installer (HookInit.cpp) after the inline detour is
 *  placed so we can forward execution to the real engine code.
 */
ServerNetworkHandler_tick_t g_originalTick = nullptr;

/*
 *  hookServerNetworkHandlerTick()
 *  ------------------------------
 *  Replacement for the game's ServerNetworkHandler::tick. This is called
 *  on every engine tick frame instead of the original.
 *
 *  Flow:
 *    1. Always call the original tick first so the game state updates
 *       normally (this preserves network sync, entity physics, etc.).
 *    2. If isPublicServer == true, bail out immediately — no mods.
 *    3. Otherwise, call executeServerSideMods() to run custom logic.
 */
void hookServerNetworkHandlerTick(void* self, float deltaTime) {
    /*
     *  Forward to the original implementation unconditionally.
     *  Without this the game loop would break — no entity movement,
     *  no network packets, no world tick.
     */
    if (g_originalTick != nullptr) {
        g_originalTick(self, deltaTime);
    }

    /*
     *  Public server guard: when connected to realms or a remote server,
     *  skip all custom mod logic to keep gameplay entirely vanilla.
     *  This prevents accidental anti-cheat triggers and respects other
     *  players' unmodified experience.
     */
    if (g_isPublicServer) {
        return;
    }

    /*
     *  Local/P2P session detected — safe to run server-side mods.
     *  The executeServerSideMods function acts as the dispatch point
     *  for all custom gameplay modifications, commands, and features.
     */
    executeServerSideMods();
}

/*
 *  executeServerSideMods()
 *  -----------------------
 *  Main pipeline for all server-side custom modifications. Runs only in
 *  local host or peer-to-peer friend sessions.
 *
 *  Uses a static flag to ensure one-shot initialization. Modify or remove
 *  this guard if you need per-frame mod logic instead.
 *
 *  Extension point: add your custom mod calls here (e.g., custom items,
 *  world generation hooks, gameplay commands, etc.).
 */
void executeServerSideMods(void) {
    static bool modsExecuted = false;
    if (modsExecuted) return;
    modsExecuted = true;

    printf("[MCCL] Local/P2P session detected — executing server-side modifications.\n");

    /*
     *  TODO: Insert custom server-side mod pipeline here.
     *  Examples:
     *    - registerCustomCommands()
     *    - applyGameplayPatches()
     *    - initializeCustomWorldGen()
     *    - spawnDebugUI()
     */
}
